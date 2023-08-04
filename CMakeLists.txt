cmake_minimum_required (VERSION 3.26)

project ("scs" VERSION 0 LANGUAGES CXX)

option(SCS_ENABLE_TESTS "Enable tests if top-level project" ON)
option(SCS_COMPILE_BENCHMARKS "Compile benchmarks" ON)
option(SCS_MEMORY_TRACKER "Track allocations" ON)
option(SCS_VERBOSE_OUTPUT "Verbose output" ON)

option(SCS_PCH "Use precompiled headers" OFF)
option(SCS_UNITY "Use unity build mode" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING YES)

#execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_COLOR_MAKEFILE on)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

message("####################################################")
message("SCS_PCH is: ${SCS_PCH}")
message("SCS_VERBOSE_OUTPUT is: ${SCS_VERBOSE_OUTPUT}")
message("####################################################")

# Preprocessor definitions
if (SCS_VERBOSE_OUTPUT)
    add_definitions(-DSCS_VERBOSE=1)
endif()

# Library
add_subdirectory(src/scs)

# Application
add_subdirectory(app)

# Examples
add_subdirectory(examples)

# Tests
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND SCS_ENABLE_TESTS)
    include(CTest)
    enable_testing()
    include(GoogleTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(tests)
endif()

# Benchmarks
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND SCS_COMPILE_BENCHMARKS)
    add_subdirectory(bench)
endif()

message(STATUS "[SCS] CXX STANDARD: ${CMAKE_CXX_STANDARD}")
