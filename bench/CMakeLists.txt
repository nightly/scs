cmake_minimum_required (VERSION 3.26)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

add_subdirectory("${PROJECT_SOURCE_DIR}/external/gbench" "external/gbench")
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}/external/gbench" APPEND
  PROPERTY COMPILE_DEFINITIONS "-DCMAKE_BUILD_TYPE=Release")

macro(package_add_benchmark BENCHNAME)
    add_executable(${BENCHNAME} ${ARGN})

    target_link_libraries(${BENCHNAME} PRIVATE 
        scs
        scs_examples
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
#   Combinatorics
# =========================

package_add_benchmark("BenchComActionInstantiations" "Combinatorics/Actions/instantiations.cpp")
package_add_benchmark("BenchComCartesianProduct" "Combinatorics/CartesianProduct/product.cpp")

# =========================
#   Situation Calculus
# =========================
package_add_benchmark("BenchScCompoundAction" "SituationCalculus/compound_action.cpp")

# =========================
#   Synthesis
# =========================
package_add_benchmark("BenchSynthHingeExpansion" "Synthesis/Hinge/expansion.cpp")
package_add_benchmark("BenchSynthHingeTopology" "Synthesis/Hinge/topology.cpp")

package_add_benchmark("BenchSynthHingeControllerQuick" "Synthesis/Hinge/controller_quick.cpp")
package_add_benchmark("BenchSynthHingeControllerFull" "Synthesis/Hinge/controller_full.cpp")
package_add_benchmark("BenchSynthHingeControllerFullLimits" "Synthesis/Hinge/controller_full_limits.cpp")
package_add_benchmark("BenchSynthHingeControllerExtended" "Synthesis/Hinge/controller_extended.cpp")


