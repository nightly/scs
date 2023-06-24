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
# add_subdirectory("${PROJECT_SOURCE_DIR}/external/json" "external/json") # public dependency of lts

# ========================
#     External (Boost)
# ========================
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/core" "external/boost/core")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/variant" "external/boost/variant")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/assert" "external/boost/assert")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/config" "external/boost/config")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/bind" "external/boost/bind")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/container_hash" "external/boost/container_hash")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/type_traits" "external/boost/type_traits")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/static_assert" "external/boost/static_assert")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/detail" "external/boost/detail")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/preprocessor" "external/boost/preprocessor")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/integer" "external/boost/integer")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/throw_exception" "external/boost/throw_exception")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/move" "external/boost/move")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/mpl" "external/boost/mpl")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/predef" "external/boost/predef")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/utility" "external/boost/utility")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/io" "external/boost/io")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/type_index" "external/boost/type_index")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/smart_ptr" "external/boost/smart_ptr")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/describe" "external/boost/describe")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/mp11" "external/boost/mp11")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/container" "external/boost/container")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/intrusive" "external/boost/intrusive")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/boost/winapi" "external/boost/winapi")

target_link_libraries(scs 
    PUBLIC 
        lts
        nlohmann_json
        spdlog::spdlog
        Boost::container # flat_map
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