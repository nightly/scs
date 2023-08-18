cmake_minimum_required (VERSION 3.26)

file(GLOB_RECURSE SCS_SOURCES ${PROJECT_SOURCE_DIR}/src "*.cpp" "*.h")
list(FILTER SCS_SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/src/scs/LTS/.*" )

add_library(scs STATIC ${SCS_SOURCES})

target_include_directories(scs
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

if (SCS_PCH)
    message("[SCS] Generating precompiled headers")
    target_precompile_headers(scs
        PUBLIC
         "${CMAKE_CURRENT_SOURCE_DIR}/Common/pch.h"
    )
endif()

if (SCS_UNITY)
    set_target_properties(scs PROPERTIES UNITY_BUILD ON)
endif()


# Preprocessor definitions
if (SCS_MEMORY_TRACKER)
    target_compile_definitions(scs PUBLIC SCS_MEMORY_TRACKER=1)
endif()


# ==================
#   Dependencies
# ==================

# ========================
#       Internal
# ========================

add_subdirectory("${PROJECT_SOURCE_DIR}/src/scs/LTS" "src/scs/LTS")

# ========================
#       External
# ========================

add_subdirectory("${PROJECT_SOURCE_DIR}/external/magic_enum" "external/magic_enum")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/spdlog" "external/spdlog")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/matchit" "external/matchit")

# Used by LTS project
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/unordered_dense" "external/unordered_dense")

# ========================
#     External (Boost)
# ========================
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/core" "external/boost/core")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/assert" "external/boost/assert")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/container" "external/boost/container")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/intrusive" "external/boost/intrusive")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/move" "external/boost/move")

# Used by LTS project
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/config" "external/boost/config")
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/container_hash" "external/boost/container_hash")
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/mp11" "external/boost/mp11")
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/describe" "external/boost/describe")
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/static_assert" "external/boost/static_assert")
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/type_traits" "external/boost/type_traits")

target_link_libraries(scs 
    PUBLIC 
        lts
        spdlog::spdlog
        Boost::container # flat_map
        Boost::container_hash
        unordered_dense::unordered_dense
        matchit
    PRIVATE
        magic_enum
)

set_target_properties(scs
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
)