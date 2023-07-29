cmake_minimum_required (VERSION 3.26)

file(GLOB_RECURSE SCS_EXAMPLES_SOURCES "${PROJECT_SOURCE_DIR}/Examples/*.cpp" "${PROJECT_SOURCE_DIR}/Examples/*.h")

add_library(scs_examples STATIC ${SCS_EXAMPLES_SOURCES})
target_link_libraries(scs_examples PUBLIC scs)
message("[SCS] Compiling Examples")

target_include_directories(scs_examples
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Examples
)
set_target_properties(scs_examples 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
)
