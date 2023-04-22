cmake_minimum_required (VERSION 3.22)

# Dependencies
add_subdirectory("${PROJECT_SOURCE_DIR}/external/gtest" "external/gtest")

# Macro
macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})

    target_compile_features(${TESTNAME}  PRIVATE cxx_std_23)
    target_link_libraries(${TESTNAME} 
    PRIVATE 
        scs
        gtest
        gmock
        gtest_main)

    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )

    set_target_properties(${TESTNAME} PROPERTIES 
        FOLDER tests
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/tests")
endmacro()

message("[SCS] Compiling tests")

# Test Suite

# =========================
#   ConGolog
# =========================

## =========================
##  Parser
## =========================

package_add_test("Lex" "ConGolog/Parser/lexer.cpp")
package_add_test("LexToken" "ConGolog/Parser/token.cpp")
package_add_test("Parser" "ConGolog/Parser/parser.cpp")
package_add_test("FolParser" "ConGolog/Parser/fol.cpp")

# Example domain
package_add_test("ParserExampleRecipe" "ConGolog/Parser/Example/recipe.cpp")
package_add_test("ParserExampleResource1" "ConGolog/Parser/Example/resource_1.cpp")
package_add_test("ParserExampleResource2" "ConGolog/Parser/Example/resource_2.cpp")
package_add_test("ParserExampleResource3" "ConGolog/Parser/Example/resource_3.cpp")
package_add_test("ParserExampleResouce4" "ConGolog/Parser/Example/resource_4.cpp")
package_add_test("ParserExampleResouce5" "ConGolog/Parser/Example/resource_5.cpp")

# Ship domain
package_add_test("ParserShipRecipe" "ConGolog/Parser/Ship/recipe.cpp")
package_add_test("ParserShipResource1" "ConGolog/Parser/Ship/resource_1.cpp")

# =========================
#   First Order Logic
# =========================

package_add_test("FolPrinter" "FirstOrderLogic/predicate.cpp")
package_add_test("FolBool" "FirstOrderLogic/bool.cpp")
package_add_test("FolPredicate" "FirstOrderLogic/printer.cpp")
package_add_test("FolBinary" "FirstOrderLogic/binary.cpp")
package_add_test("FolUnary" "FirstOrderLogic/unary.cpp")
package_add_test("FolQuantifier" "FirstOrderLogic/quantifier.cpp")
package_add_test("FolObject" "FirstOrderLogic/object.cpp")
package_add_test("FolEquality" "FirstOrderLogic/equality.cpp")

# =========================
#   Situation Calculus
# =========================

package_add_test("ScRelationalFluent" "SituationCalculus/relational_fluent.cpp")
package_add_test("ScSituation" "SituationCalculus/situation.cpp")
package_add_test("ScInitialSituation" "SituationCalculus/initial_situation.cpp")
package_add_test("ScPoss" "SituationCalculus/poss.cpp")
package_add_test("ScSuccessor" "SituationCalculus/successor.cpp")

# Domains

## Blocks world
package_add_test("ScBlocksPoss" "SituationCalculus/Domains/Blocks/poss.cpp")
package_add_test("ScBlocksSsa" "SituationCalculus/Domains/Blocks/ssa.cpp")

## Ship
package_add_test("ScShipPoss" "SituationCalculus/Domains/Ship/poss.cpp")
package_add_test("ScShipSsa" "SituationCalculus/Domains/Ship/ssa.cpp")

# Database domain
package_add_test("ScDatabaseSsa" "SituationCalculus/Domains/Database/ssa.cpp")

# =========================
#   Synthesis
# =========================




# =========================
#   Common
# =========================

package_add_test("Io" "Common/io.cpp")
package_add_test("Strings" "Common/strings.cpp")