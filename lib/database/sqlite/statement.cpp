// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 - 2025 Pionix GmbH and Contributors to EVerest

#include <iostream>

#include <database/exceptions.hpp>
#include <database/sqlite/statement.hpp>

namespace everest::db::sqlite {

Statement::Statement(sqlite3* db, const std::string& query) : db(db), stmt(nullptr) {
    if (sqlite3_prepare_v2(db, query.c_str(), query.size(), &this->stmt, nullptr) != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << std::endl;
        throw QueryExecutionException("Could not prepare statement for database.");
    }
}

Statement::~Statement() {
    if (this->stmt != nullptr) {
        if (sqlite3_finalize(this->stmt) != SQLITE_OK) {
            std::cout << "Error finalizing statement: " << sqlite3_errmsg(this->db) << std::endl;
        }
    }
}

int Statement::step() {
    return sqlite3_step(this->stmt);
}

int Statement::reset() {
    return sqlite3_reset(this->stmt);
}

int Statement::changes() {
    // Rows affected by the last INSERT, UPDATE, DELETE
    return sqlite3_changes(this->db);
}

int Statement::bind_text(const int idx, const std::string& val, SQLiteString lifetime) {
    return sqlite3_bind_text(this->stmt, idx, val.c_str(), val.length(),
                             lifetime == SQLiteString::Static ? SQLITE_STATIC : SQLITE_TRANSIENT);
}

int Statement::bind_text(const std::string& param, const std::string& val, SQLiteString lifetime) {
    int index = sqlite3_bind_parameter_index(this->stmt, param.c_str());
    if (index <= 0) {
        throw std::out_of_range("Parameter not found in SQL query");
    }
    return bind_text(index, val, lifetime);
}

int Statement::bind_int(const int idx, const int val) {
    return sqlite3_bind_int(this->stmt, idx, val);
}

int Statement::bind_int(const std::string& param, const int val) {
    int index = sqlite3_bind_parameter_index(this->stmt, param.c_str());
    if (index <= 0) {
        throw std::out_of_range("Parameter not found in SQL query");
    }
    return bind_int(index, val);
}

int Statement::bind_int64(const int idx, const int64_t val) {
    return sqlite3_bind_int64(this->stmt, idx, val);
}

int Statement::bind_int64(const std::string& param, const int64_t val) {
    int index = sqlite3_bind_parameter_index(this->stmt, param.c_str());
    if (index <= 0) {
        throw std::out_of_range("Parameter not found in SQL query");
    }
    return bind_int64(index, val);
}

int Statement::bind_double(const int idx, const double val) {
    return sqlite3_bind_double(this->stmt, idx, val);
}

int Statement::bind_double(const std::string& param, const double val) {
    int index = sqlite3_bind_parameter_index(this->stmt, param.c_str());
    if (index <= 0) {
        throw std::out_of_range("Parameter not found in SQL query");
    }
    return bind_double(index, val);
}

int Statement::bind_null(const int idx) {
    return sqlite3_bind_null(this->stmt, idx);
}

int Statement::bind_null(const std::string& param) {
    int index = sqlite3_bind_parameter_index(this->stmt, param.c_str());
    if (index <= 0) {
        throw std::out_of_range("Parameter not found in SQL query");
    }
    return bind_null(index);
}

int Statement::get_number_of_rows() {
    return sqlite3_data_count(this->stmt);
}

int Statement::column_type(const int idx) {
    return sqlite3_column_type(this->stmt, idx);
}

std::string Statement::column_text(const int idx) {
    return reinterpret_cast<const char*>(sqlite3_column_text(this->stmt, idx));
}

std::optional<std::string> Statement::column_text_nullable(const int idx) {
    auto p = sqlite3_column_text(this->stmt, idx);
    if (p != nullptr) {
        return reinterpret_cast<const char*>(p);
    } else {
        return std::optional<std::string>{};
    }
}

int Statement::column_int(const int idx) {
    return sqlite3_column_int(this->stmt, idx);
}

int64_t Statement::column_int64(const int64_t idx) {
    return sqlite3_column_int64(this->stmt, idx);
}

double Statement::column_double(const int idx) {
    return sqlite3_column_double(this->stmt, idx);
}

} // namespace everest::db::sqlite