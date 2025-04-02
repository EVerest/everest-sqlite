
add_library(everest_sqlite)
add_library(everest::sqlite ALIAS everest_sqlite)

target_sources(everest_sqlite
    PRIVATE
        everest_sqlite/database/sqlite/statement.cpp
        everest_sqlite/database/sqlite/connection.cpp
        everest_sqlite/database/sqlite/schema_updater.cpp
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

target_compile_features(everest_sqlite PRIVATE cxx_std_17)
