cmake_minimum_required (VERSION 3.26)

add_executable(scs_cli scs.cpp)
target_link_libraries(scs_cli PRIVATE scs scs_examples)

message("[SCS] Compiling CLI")

set_target_properties(scs_cli 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

get_property(cli_compiler_flags TARGET scs_cli PROPERTY COMPILE_OPTIONS)
get_property(cli_link_flags TARGET scs_cli PROPERTY LINK_FLAGS)
message("Compiler flags for SCS CLI: ${cli_compiler_flags}")
message("Link flags for SCS CLI: ${cli_link_flags}")

# if (SCS_UNITY)
#    set_target_properties(scs_cli PROPERTIES UNITY_BUILD ON)
# endif()

target_link_libraries(scs_cli
    PRIVATE
        magic_enum
)

if (SCS_BUILD_PAPER_RUNNER)
    add_library(scs_paper_experiments STATIC
        paper/experiments.cpp
        paper/paper.cpp
        paper/process.cpp)
    target_include_directories(scs_paper_experiments PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/paper")
    target_link_libraries(scs_paper_experiments
        PUBLIC scs scs_examples benchmark::benchmark)
    target_compile_definitions(scs_paper_experiments PRIVATE
        SCS_BUILD_TYPE="${CMAKE_BUILD_TYPE}")

    find_package(Git QUIET)
    if (Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short=12 HEAD
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            OUTPUT_VARIABLE SCS_GIT_COMMIT_VALUE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if (SCS_GIT_COMMIT_VALUE)
            target_compile_definitions(scs_paper_experiments PRIVATE
                SCS_GIT_COMMIT="${SCS_GIT_COMMIT_VALUE}")
        endif()
    endif()

    add_executable(scs_paper paper/main.cpp)
    target_link_libraries(scs_paper PRIVATE scs_paper_experiments)
    set_target_properties(scs_paper PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()
