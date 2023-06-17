cmake_minimum_required (VERSION 3.26)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

add_subdirectory("${PROJECT_SOURCE_DIR}/external/gbench" "external/gbench")
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}/external/gbench" APPEND
  PROPERTY COMPILE_DEFINITIONS "-DCMAKE_BUILD_TYPE=Release")

macro(package_add_benchmark BENCHNAME)
    add_executable(${BENCHNAME} ${ARGN})
    target_compile_features(${BENCHNAME} PRIVATE cxx_std_20)

    target_link_libraries(${BENCHNAME} PRIVATE 
        scs
        benchmark::benchmark
        benchmark::benchmark_main)


    set_target_properties(${BENCHNAME} PROPERTIES 
        FOLDER Benchmarks
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Benchmarks")
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