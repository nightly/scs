if (NOT DEFINED SCS_PAPER_EXECUTABLE OR NOT DEFINED SCS_PAPER_TEMP_DIR)
    message(FATAL_ERROR "worker smoke test requires executable and temporary directory")
endif()

file(MAKE_DIRECTORY "${SCS_PAPER_TEMP_DIR}")
set(worker_output "${SCS_PAPER_TEMP_DIR}/astar-worker.tsv")
file(REMOVE "${worker_output}")

execute_process(
    COMMAND "${SCS_PAPER_EXECUTABLE}" --internal-worker astar3
            --output "${worker_output}" --timeout-ms 1
    RESULT_VARIABLE worker_result
    OUTPUT_QUIET
    ERROR_QUIET)

if (NOT worker_result EQUAL 0)
    message(FATAL_ERROR "paper A* worker returned ${worker_result}")
endif()
if (NOT EXISTS "${worker_output}")
    message(FATAL_ERROR "paper A* worker did not create its TSV snapshot")
endif()

file(READ "${worker_output}" contents)
if (NOT contents MATCHES "status.iterations.cpu_seconds")
    message(FATAL_ERROR "paper A* worker TSV header is invalid")
endif()
if (NOT contents MATCHES "timeout.1")
    message(FATAL_ERROR "paper A* worker did not report its timeout")
endif()

set(nonempty "${SCS_PAPER_TEMP_DIR}/nonempty")
file(MAKE_DIRECTORY "${nonempty}")
file(WRITE "${nonempty}/sentinel" "do not overwrite")
execute_process(
    COMMAND "${SCS_PAPER_EXECUTABLE}" --output-dir "${nonempty}" --suite grounding
    RESULT_VARIABLE overwrite_result
    OUTPUT_QUIET
    ERROR_QUIET)
if (overwrite_result EQUAL 0)
    message(FATAL_ERROR "paper runner accepted a non-empty output directory")
endif()
