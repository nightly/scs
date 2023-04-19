cmake_minimum_required (VERSION 3.22)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

add_subdirectory("${PROJECT_SOURCE_DIR}/external/gbench" "external/gbench")
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}/external/gbench" APPEND
  PROPERTY COMPILE_DEFINITIONS "-DCMAKE_BUILD_TYPE=Release")

macro(package_add_benchmark BENCHNAME)
    add_executable(${BENCHNAME} ${ARGN})
    target_compile_features(${BENCHNAME}  PRIVATE cxx_std_23)

    target_link_libraries(${BENCHNAME} PRIVATE 
        scs
        benchmark::benchmark
        benchmark::benchmark_main)


    set_target_properties(${BENCHNAME} PROPERTIES 
        FOLDER benchmarks
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}"
    )
endmacro()

# Benchmark Suite
message("[SCS] Compiling benchmarks")

# =========================
#   First Order Logic
# =========================

package_add_benchmark("BenchFolEquality" "FirstOrderLogic/equality.cpp")

# =========================
#   Synthesis
# =========================