cmake_minimum_required (VERSION 3.26)

file(GLOB_RECURSE SCS_CLI_SOURCES "scs.cpp" ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.h")

add_executable(scs_cli ${SCS_CLI_SOURCES})
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