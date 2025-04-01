// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 - 2025 Pionix GmbH and Contributors to EVerest

#include <database/sqlite/schema_updater.hpp>

#include <fstream>
#include <iostream>
#include <regex>

namespace everest::db::sqlite {

enum class Direction {
    Up,
    Down
};

struct MigrationFile {
    fs::path path;
    uint32_t version;
    Direction direction;
};

std::ostream& operator<<(std::ostream& os, const MigrationFile& info) {
    os << "Migration file [" << (info.direction == Direction::Up ? "up" : "down") << "] version " << info.version
       << ", path: " << info.path.c_str();
    return os;
}

std::vector<MigrationFile> get_migration_file_list(const fs::path& migration_file_directory) {
    std::regex filename_pattern{R"(^(\d+)_(up|down)(-[ \S]+|)\.sql$)"};

    std::vector<MigrationFile> result;

    for (auto& entry : fs::directory_iterator(migration_file_directory)) {
        if (entry.is_regular_file() and entry.file_size() > 0) {
            const fs::path& path = entry.path();
            std::cmatch match;
            std::string filename =
                path.filename(); // Store in a variable otherwise after the match the string temporary is gone
            if (std::regex_match(filename.c_str(), match, filename_pattern) and match.size() == 4) {
                // [0] = whole match
                // [1] = version id
                // [2] = up or down
                // [3] = description or empty
                result.push_back(MigrationFile{path, static_cast<uint32_t>(std::stoul(match[1].str())),
                                               match[2] == "up" ? Direction::Up : Direction::Down});
            }
        }
    }

    return result;
}

void filter_and_sort_migration_file_list(std::vector<MigrationFile>& list, Direction direction, uint32_t min_version,
                                         uint32_t max_version) {
    auto filter = [direction, min_version, max_version](const MigrationFile& item) {
        return item.direction != direction or item.version < min_version or item.version > max_version;
    };

    list.erase(std::remove_if(list.begin(), list.end(), filter), list.end());

    std::sort(list.begin(), list.end(), [direction](const auto& a, const auto& b) {
        if (direction == Direction::Up) {
            return a.version < b.version;
        } else {
            return b.version < a.version;
        }
    });
}

bool is_migration_file_list_valid(std::vector<MigrationFile>& list, uint32_t max_version) {
    auto expected_files = (max_version * 2) - 1;
    if (list.size() < expected_files) {
        std::cout << "Expected " << expected_files << " files but only found: " << list.size() << std::endl;
        return false;
    }
    if (list.size() % 2 == 0) {
        std::cout << "Nr of migration files should always be uneven: 1 initial file + n pairs" << std::endl;
        return false;
    }

    std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
        return std::tie(a.version, a.direction) < std::tie(b.version, b.direction);
    });

    if (list.at(0).version != 1 or list.at(0).direction != Direction::Up) {
        std::cout << "Invalid initial migration file" << std::endl;
        return false;
    }

    for (size_t i = 1; i < list.size(); i += 2) {
        uint32_t expected_version = (i / 2) + 2;
        const auto& up = list.at(i);
        const auto& down = list.at(i + 1);
        if (up.version != expected_version || up.direction != Direction::Up) {
            std::cout << "Expected migration file " << expected_version << "_up.sql but got: " << up.path.filename()
                      << std::endl;
            return false;
        }
        if (down.version != expected_version || down.direction != Direction::Down) {
            std::cout << "Expected migration file " << expected_version << "_down.sql but got: " << down.path.filename()
                      << std::endl;
            return false;
        }
    }
    return true;
}

std::optional<std::vector<MigrationFile>> get_migration_file_sequence(const fs::path& migration_file_directory,
                                                                      Direction direction, uint32_t current_version,
                                                                      uint32_t target_version) {
    auto list = get_migration_file_list(migration_file_directory);

    std::cout << "Migration list:" << std::endl;

    for (auto& item : list) {
        std::cout << item << std::endl;
    }

    if (!is_migration_file_list_valid(list, std::max(current_version, target_version))) {
        return std::nullopt;
    }

    const auto lowest = std::min(current_version, target_version) + 1;
    const auto highest = std::max(current_version, target_version);

    filter_and_sort_migration_file_list(list, direction, lowest, highest);

    std::cout << "Migration files to apply:" << std::endl;

    for (auto& item : list) {
        std::cout << item << std::endl;
    }
    return list;
}

SchemaUpdater::SchemaUpdater(ConnectionInterface* database) noexcept : database(database) {
}

bool SchemaUpdater::apply_migration_files(const fs::path& migration_file_directory,
                                                  uint32_t target_schema_version) {
    if (!fs::is_directory(migration_file_directory)) {
        std::cout << "Migration files must be in a directory: " << migration_file_directory.c_str() << std::endl;
        return false;
    }

    if (target_schema_version == 0) {
        std::cout << "Migration target_version 0 is invalid" << std::endl;
        return false;
    }

    uint32_t current_version = 0;

    try {
        this->database->open_connection();
        current_version = this->database->get_user_version();
        std::cout << "Target version: " << target_schema_version << ", current version: " << current_version
                  << std::endl;
    } catch (std::runtime_error& e) {
        std::cout << "Failure during migration file apply: " << e.what() << std::endl;
        return false;
    }

    if (current_version == target_schema_version) {
        std::cout << "No migrations to apply since versions match" << std::endl;
        this->database->close_connection();
        return true;
    }

    Direction direction = Direction::Up;

    if (current_version > target_schema_version) {
        direction = Direction::Down;
    }

    auto list =
        get_migration_file_sequence(migration_file_directory, direction, current_version, target_schema_version);

    if (!list.has_value()) {
        std::cout << "Missing migration files in sequence, no actions performed" << std::endl;
        this->database->close_connection();
        return false;
    }

    bool retval = true;
    try {
        auto transaction = this->database->begin_transaction();

        for (const auto& item : list.value()) {
            std::ifstream stream{item.path};
            std::stringstream init_sql;

            init_sql << stream.rdbuf();

            if (!this->database->execute_statement(init_sql.str())) {
                std::cout << "Could not apply migration file " << item.path << std::endl;
                throw std::runtime_error("Database access error");
            }
        }

        this->database->set_user_version(target_schema_version);
        transaction->commit();
    } catch (std::exception& e) {
        std::cout << "Failure during migration file apply: " << e.what() << std::endl;
        retval = false;
    }

    this->database->close_connection();
    return retval;
}

} // namespace everest::db::sqlite