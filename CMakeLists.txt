cmake_minimum_required(VERSION 3.14)
project(datapainter VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for LSP/IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type defaults to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find required dependencies
find_package(SQLite3 REQUIRED)

# Platform-specific terminal handling
if(UNIX)
    # ncurses or direct termios on Unix-like systems
    find_package(Curses)
    if(NOT CURSES_FOUND)
        message(STATUS "ncurses not found, will use termios directly")
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(DATAPAINTER_SOURCES
    src/main.cpp
    src/database.cpp
    src/argument_parser.cpp
    src/metadata.cpp
    src/data_table.cpp
    src/unsaved_changes.cpp
    src/viewport.cpp
    # UI layer will go here:
    # src/terminal.cpp
    # etc.
)

# Main executable
add_executable(datapainter ${DATAPAINTER_SOURCES})

# Link libraries
target_link_libraries(datapainter PRIVATE SQLite::SQLite3)
if(CURSES_FOUND)
    target_link_libraries(datapainter PRIVATE ${CURSES_LIBRARIES})
endif()

# Install target
install(TARGETS datapainter DESTINATION bin)

# Testing
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()

    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Test sources
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/test_database.cpp
        tests/test_metadata_table.cpp
        tests/test_unsaved_changes_table.cpp
        tests/test_table_validation.cpp
        tests/test_argument_parser.cpp
        tests/test_metadata.cpp
        tests/test_data_table.cpp
        tests/test_unsaved_changes.cpp
        tests/test_viewport.cpp
        # Implementation files needed by tests
        src/database.cpp
        src/argument_parser.cpp
        src/metadata.cpp
        src/data_table.cpp
        src/unsaved_changes.cpp
        src/viewport.cpp
        # More test files will be added as we build
    )

    # Test executable
    add_executable(datapainter_tests ${TEST_SOURCES})
    target_link_libraries(datapainter_tests PRIVATE
        gtest_main
        gmock
        SQLite::SQLite3
    )
    target_include_directories(datapainter_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

    # Discover tests
    include(GoogleTest)
    gtest_discover_tests(datapainter_tests)
endif()

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print configuration summary
message(STATUS "=== DataPainter Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "SQLite3 found: ${SQLite3_FOUND}")
if(CURSES_FOUND)
    message(STATUS "Curses found: ${CURSES_FOUND}")
endif()
message(STATUS "================================")
