cmake_minimum_required (VERSION 3.26)

# Dependencies
add_subdirectory("${PROJECT_SOURCE_DIR}/external/gtest" "external/gtest")

# Macro
macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})

    target_link_libraries(${TESTNAME} 
    PRIVATE 
        scs
        scs_examples
        gtest
        gmock
        gtest_main)

    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )

    set_target_properties(${TESTNAME} PROPERTIES 
        FOLDER Tests
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Tests")
endmacro()

message("[SCS] Compiling tests")

# Test Suite

# =========================
#   ConGolog
# =========================

#=============#
#  Parser     #
#=============#

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

#=============#
# Char Graphs #
#=============#

package_add_test("ConGologCgHinge" "ConGolog/CharacteristicGraph/hinge.cpp")

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
package_add_test("FolCoopMatrix" "FirstOrderLogic/coop.cpp")

# =========================
#   Situation Calculus
# =========================

package_add_test("ScRelationalFluent" "SituationCalculus/relational_fluent.cpp")
package_add_test("ScSituation" "SituationCalculus/situation.cpp")
package_add_test("ScBat" "SituationCalculus/bat.cpp")
package_add_test("ScPoss" "SituationCalculus/poss.cpp")
package_add_test("ScSuccessor" "SituationCalculus/successor.cpp")
package_add_test("ScCoopMatrix" "SituationCalculus/coop_matrix.cpp")

# Domains
#=======#

## Blocks world
package_add_test("ScBlocksPoss" "SituationCalculus/Domains/Blocks/poss.cpp")
package_add_test("ScBlocksSsa" "SituationCalculus/Domains/Blocks/ssa.cpp")

## Ship
package_add_test("ScShipPoss" "SituationCalculus/Domains/Ship/poss.cpp")
package_add_test("ScShipSsa" "SituationCalculus/Domains/Ship/ssa.cpp")
package_add_test("ScShipBoundedSsa" "SituationCalculus/Domains/Ship/bounded_ssa.cpp")

## Database domain
package_add_test("ScDatabaseSimpleSsa" "SituationCalculus/Domains/Database/simple_ssa.cpp")


# =========================
#   Combinatorics
# =========================
package_add_test("ComActionInstantiations" "Combinatorics/action_instantiations.cpp")
package_add_test("ComCartesianProducts" "Combinatorics/cartesian_product.cpp")



# =========================
#   Synthesis
# =========================
#=============#
#  Cache      #
#=============#
package_add_test("SynthActionsCache" "Synthesis/Actions/Cache.cpp")

#=============#
#  Examples   #
#=============#
package_add_test("SyntExampleHingePoss" "Synthesis/Examples/Hinge/poss.cpp")
package_add_test("SyntExampleHingeRecipe" "Synthesis/Examples/Hinge/recipe.cpp")

package_add_test("SyntExampleHingeQuickController" "Synthesis/Examples/HingeQuick/controller.cpp")

#=============#
#  Topology   #
#=============#
package_add_test("TopologyShuffleTransitions" "Synthesis/Topology/shuffle_transitions.cpp")

# =========================
#   Common
# =========================

package_add_test("Io" "Common/io.cpp")
package_add_test("Strings" "Common/strings.cpp")