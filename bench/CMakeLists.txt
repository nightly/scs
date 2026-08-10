cmake_minimum_required (VERSION 3.26)

macro(package_add_benchmark BENCHNAME)
    add_executable(${BENCHNAME} ${ARGN})

    target_compile_definitions(${BENCHNAME} PRIVATE MEMORY_PROFILER)

    target_link_libraries(${BENCHNAME} PRIVATE 
        scs
        scs_examples
        benchmark::benchmark
        benchmark::benchmark_main)

    target_include_directories(${BENCHNAME} PRIVATE 
       ${CMAKE_CURRENT_SOURCE_DIR})

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
#   Combinatorics
# =========================

package_add_benchmark("BenchComActionInstantiations" "Combinatorics/Actions/instantiations.cpp")
package_add_benchmark("BenchComCartesianProduct" "Combinatorics/CartesianProduct/product.cpp")

# =========================
#   Situation Calculus
# =========================
package_add_benchmark("BenchScCompoundAction" "SituationCalculus/compound_action.cpp")
