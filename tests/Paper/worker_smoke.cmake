if (NOT DEFINED SCS_PAPER_EXECUTABLE OR NOT DEFINED SCS_PAPER_TEMP_DIR)
    message(FATAL_ERROR "paper smoke test requires executable and temporary directory")
endif()

file(REMOVE_RECURSE "${SCS_PAPER_TEMP_DIR}")
execute_process(
    COMMAND "${SCS_PAPER_EXECUTABLE}" --suite smoke --output-dir "${SCS_PAPER_TEMP_DIR}"
    RESULT_VARIABLE result)
if (NOT result EQUAL 0)
    message(FATAL_ERROR "exact paper smoke suite returned ${result}")
endif()

foreach(required metrics.tsv faithful-breadth-first-controller.gv faithful-breadth-first-controller.tex)
    if (NOT EXISTS "${SCS_PAPER_TEMP_DIR}/${required}")
        message(FATAL_ERROR "paper smoke suite did not create ${required}")
    endif()
endforeach()

file(READ "${SCS_PAPER_TEMP_DIR}/metrics.tsv" contents)
if (NOT contents MATCHES "faithful-breadth-first.winning.10")
    message(FATAL_ERROR "paper smoke metrics do not report the expected K*=10 result")
endif()

set(nonempty "${SCS_PAPER_TEMP_DIR}/nonempty")
file(MAKE_DIRECTORY "${nonempty}")
file(WRITE "${nonempty}/sentinel" "do not overwrite")
execute_process(
    COMMAND "${SCS_PAPER_EXECUTABLE}" --output-dir "${nonempty}" --suite smoke
    RESULT_VARIABLE overwrite_result
    OUTPUT_QUIET
    ERROR_QUIET)
if (overwrite_result EQUAL 0)
    message(FATAL_ERROR "paper runner accepted a non-empty output directory")
endif()
