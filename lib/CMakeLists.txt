
add_library(everest_sqlite)
add_library(everest::sqlite ALIAS everest_sqlite)

target_sources(everest_sqlite
    PRIVATE
        database/sqlite/statement.cpp
        database/sqlite/connection.cpp
        database/sqlite/schema_updater.cpp
)

target_link_libraries(everest_sqlite
    PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
)

target_include_directories(everest_sqlite
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
