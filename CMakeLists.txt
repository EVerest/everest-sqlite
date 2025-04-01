cmake_minimum_required(VERSION 3.14)

project(everest-sqlite VERSION 0.1
        DESCRIPTION "SQLite wrapper"
		LANGUAGES CXX C
)

if((${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME} OR ${PROJECT_NAME}_BUILD_TESTING) AND BUILD_TESTING)
    set(EVEREST_SQLITE_BUILD_TESTING ON)
endif()

find_package(SQLite3 REQUIRED)

if (NOT DISABLE_EDM)
    find_package(everest-cmake 0.1 REQUIRED
        PATHS ../everest-cmake
    )
    if (EVEREST_SQLITE_BUILD_TESTING)
        evc_include(CodeCoverage)
        append_coverage_compiler_flags()
    endif()
    evc_setup_edm()
else()
    find_package(nlohmann_json REQUIRED)
    if (EVEREST_SQLITE_BUILD_TESTING)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/release-1.12.1.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()
endif()


# options
option(${PROJECT_NAME}_BUILD_TESTING "Build unit tests, used if included as dependency" OFF)
option(BUILD_TESTING "Build unit tests, used if standalone project" OFF)

add_subdirectory(lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(EVEREST_SQLITE_BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
    set(CMAKE_BUILD_TYPE Debug)
endif()