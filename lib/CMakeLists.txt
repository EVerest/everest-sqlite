
add_library(sqlite)
add_library(everest::sqlite ALIAS sqlite)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

target_sources(sqlite
    PRIVATE
        database/sqlite/sqlite_statement.cpp
        database/sqlite/sqlite_connection.cpp
        database/sqlite/sqlite_schema_updater.cpp
)

target_link_libraries(sqlite
    PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
)

target_include_directories(sqlite
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
