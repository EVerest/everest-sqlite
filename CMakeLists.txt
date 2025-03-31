cmake_minimum_required(VERSION 3.14)

project(libsqlite VERSION 0.1
        DESCRIPTION "SQLite wrapper"
		LANGUAGES CXX C
)

find_package(everest-cmake 0.1 REQUIRED
    PATHS ../everest-cmake
)

# dependencies
if (NOT DISABLE_EDM)
    evc_setup_edm()
    # In EDM mode, we can't install exports (because the dependencies usually do not install their exports)
    set(SQLITE_INSTALL OFF)
endif()

# options
option(${PROJECT_NAME}_BUILD_TESTING "Build unit tests, used if included as dependency" OFF)
option(BUILD_TESTING "Build unit tests, used if standalone project" OFF)
option(LIBSQLITE_INSTALL "Install the library (shared data might be installed anyway)" ${EVC_MAIN_PROJECT})

if((${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME} OR ${PROJECT_NAME}_BUILD_TESTING) AND BUILD_TESTING)
    set(LIBSQLITE_BUILD_TESTING ON)
    evc_include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

add_subdirectory(lib)

if(LIBSQLITE_BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
    set(CMAKE_BUILD_TYPE Debug)
endif()