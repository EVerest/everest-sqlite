
add_library(sqlite)
add_library(everest::sqlite ALIAS sqlite)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

target_sources(sqlite
    PRIVATE
        database/sqlite/statement.cpp
        database/sqlite/connection.cpp
        database/sqlite/schema_updater.cpp
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
