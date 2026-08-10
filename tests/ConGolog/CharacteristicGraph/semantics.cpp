#include <gtest/gtest.h>

#include <unordered_set>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/export.h"
#include "scs/ConGolog/Program/programs.h"

using namespace scs;

namespace {

	CharacteristicGraph Compile(const IProgram& program) {
		return CharacteristicGraph{program.clone(), ProgramType::Recipe};
	}

	std::vector<std::string> ActionNames(const CompoundAction& action) {
		std::vector<std::string> names;
		for (const auto& component : action.Actions()) {
			names.emplace_back(component.name);
		}
		return names;
	}

}

TEST(CharacteristicGraphSemantics, PrimitiveActionAndTestFinality) {
	const auto action_graph = Compile(ActionProgram{Action{"a"}});
	EXPECT_EQ(action_graph.lts.NumOfStates(), 2);
	EXPECT_EQ(action_graph.lts.initial_state().final_condition, Formula{false});
	ASSERT_EQ(action_graph.lts.at(action_graph.lts.initial_state()).transitions().size(), 1);
	EXPECT_EQ(action_graph.lts.at(action_graph.lts.initial_state()).transitions()[0].to().final_condition, Formula{true});

	const Formula condition = Predicate{"ready"};
	const auto test_graph = Compile(Check{condition});
	EXPECT_EQ(test_graph.lts.NumOfStates(), 1);
	EXPECT_EQ(test_graph.lts.initial_state().final_condition, condition);
	EXPECT_TRUE(test_graph.lts.at(test_graph.lts.initial_state()).transitions().empty());
}

TEST(CharacteristicGraphSemantics, NilCompoundActionsLoopAliasAndOrderedPick) {
	const auto nil = Compile(Nil{});
	EXPECT_EQ(nil.lts.NumOfStates(), 1);
	EXPECT_EQ(nil.lts.initial_state().final_condition, Formula{true});
	EXPECT_TRUE(nil.lts.at(nil.lts.initial_state()).transitions().empty());

	const CompoundAction compound{std::vector<Action>{Action{"a"}, Action{"b"}}};
	const auto compound_graph = Compile(ActionProgram{compound});
	const auto& compound_edge = compound_graph.lts.at(compound_graph.lts.initial_state()).transitions()[0];
	EXPECT_EQ(ActionNames(compound_edge.label().act), (std::vector<std::string>{"a", "b"}));

	const auto iteration = Compile(Iteration{ActionProgram{Action{"a"}}});
	const auto loop = Compile(Loop{ActionProgram{Action{"a"}}});
	EXPECT_EQ(iteration.lts, loop.lts);

	const std::unordered_set<Variable> unordered{Variable{"z"}, Variable{"a"}};
	const Pick picked{unordered, ActionProgram{Action{"pair", {Variable{"a"}, Variable{"z"}}}}};
	const auto pick_graph = Compile(picked);
	const auto& variables = pick_graph.lts.at(pick_graph.lts.initial_state()).transitions()[0].label().vars;
	ASSERT_EQ(variables.size(), 2);
	EXPECT_TRUE(variables[0].name().ends_with("_a"));
	EXPECT_TRUE(variables[1].name().ends_with("_z"));
}

TEST(CharacteristicGraphSemantics, SequenceUsesLeftFinalityAsGuard) {
	const Formula condition = Predicate{"ready"};
	const auto graph = Compile(Sequence{Check{condition}, ActionProgram{Action{"a"}}});
	const auto& transitions = graph.lts.at(graph.lts.initial_state()).transitions();

	ASSERT_EQ(transitions.size(), 1);
	EXPECT_EQ(transitions[0].label().condition, condition);
	EXPECT_EQ(ActionNames(transitions[0].label().act), std::vector<std::string>{"a"});
}

TEST(CharacteristicGraphSemantics, BranchMayTerminateOrTakeEitherOperand) {
	const auto graph = Compile(Branch{Nil{}, ActionProgram{Action{"a"}}});

	EXPECT_EQ(graph.lts.initial_state().final_condition, Formula{true});
	ASSERT_EQ(graph.lts.at(graph.lts.initial_state()).transitions().size(), 1);
	EXPECT_EQ(ActionNames(graph.lts.at(graph.lts.initial_state()).transitions()[0].label().act),
		std::vector<std::string>{"a"});
}

TEST(CharacteristicGraphSemantics, PickBindingSurvivesAcrossResidualProgram) {
	const Action first{"a", {Variable{"x"}}};
	const Action second{"b", {Variable{"x"}}};
	const Pick program{{Variable{"x"}}, Sequence{ActionProgram{first}, ActionProgram{second}}};
	const auto graph = Compile(program);
	const auto& first_edge = graph.lts.at(graph.lts.initial_state()).transitions()[0];

	ASSERT_EQ(first_edge.label().vars.size(), 1);
	ASSERT_EQ(first_edge.to().live_variables.size(), 1);
	EXPECT_EQ(first_edge.label().vars[0], first_edge.to().live_variables[0]);
	const auto& second_edge = graph.lts.at(first_edge.to()).transitions()[0];
	EXPECT_TRUE(second_edge.label().vars.empty());
	ASSERT_EQ(second_edge.label().act.Actions().size(), 1);
	ASSERT_EQ(second_edge.label().act.Actions()[0].terms.size(), 1);
	EXPECT_EQ(std::get<Variable>(second_edge.label().act.Actions()[0].terms[0]), first_edge.label().vars[0]);
	EXPECT_TRUE(second_edge.to().live_variables.empty());
}

TEST(CharacteristicGraphSemantics, PickFinalityIsExistential) {
	const Pick program{{Variable{"x"}}, Check{Predicate{"ready", {Variable{"x"}}}}};
	const auto graph = Compile(program);
	const auto* quantified = std::get_if<Box<Quantifier>>(&graph.lts.initial_state().final_condition);
	ASSERT_NE(quantified, nullptr);
	EXPECT_EQ((*quantified)->kind(), QuantifierKind::Existential);
	const auto& predicate = std::get<Predicate>((*quantified)->child());
	EXPECT_EQ(std::get<Variable>(predicate.terms()[0]), (*quantified)->variable());
}

TEST(CharacteristicGraphSemantics, NestedPickShadowingUsesDistinctVariables) {
	const Pick inner{{Variable{"x"}}, ActionProgram{Action{"b", {Variable{"x"}}}}};
	const Pick outer{{Variable{"x"}}, Sequence{ActionProgram{Action{"a", {Variable{"x"}}}}, inner}};
	const auto graph = Compile(outer);
	const auto& outer_edge = graph.lts.at(graph.lts.initial_state()).transitions()[0];
	const auto& inner_edge = graph.lts.at(outer_edge.to()).transitions()[0];

	ASSERT_EQ(outer_edge.label().vars.size(), 1);
	ASSERT_EQ(inner_edge.label().vars.size(), 1);
	EXPECT_NE(outer_edge.label().vars[0], inner_edge.label().vars[0]);
}

TEST(CharacteristicGraphSemantics, IterationIsFinalAndRestartsBody) {
	const auto graph = Compile(Iteration{ActionProgram{Action{"a"}}});

	EXPECT_EQ(graph.lts.NumOfStates(), 1);
	EXPECT_EQ(graph.lts.initial_state().final_condition, Formula{true});
	ASSERT_EQ(graph.lts.at(graph.lts.initial_state()).transitions().size(), 1);
	EXPECT_EQ(graph.lts.at(graph.lts.initial_state()).transitions()[0].to(), graph.lts.initial_state());
}

TEST(CharacteristicGraphSemantics, InterleavingAdvancesExactlyOneSide) {
	const Sequence left{ActionProgram{Action{"a"}}, ActionProgram{Action{"b"}}};
	const auto graph = Compile(Interleaved{left, ActionProgram{Action{"c"}}});
	const auto& initial = graph.lts.at(graph.lts.initial_state()).transitions();

	ASSERT_EQ(initial.size(), 2);
	EXPECT_EQ(ActionNames(initial[0].label().act), std::vector<std::string>{"a"});
	EXPECT_EQ(ActionNames(initial[1].label().act), std::vector<std::string>{"c"});
	EXPECT_EQ(graph.lts.NumOfStates(), 6);
}

TEST(CharacteristicGraphSemantics, SimultaneousPairsStepsAndNeverAdvancesUnilaterally) {
	const Sequence left{ActionProgram{Action{"a"}}, ActionProgram{Action{"b"}}};
	const Sequence right{ActionProgram{Action{"c"}}, ActionProgram{Action{"d"}}};
	const auto graph = Compile(Simultaneous{left, right});
	const auto& first = graph.lts.at(graph.lts.initial_state()).transitions();

	ASSERT_EQ(first.size(), 1);
	EXPECT_EQ(ActionNames(first[0].label().act), (std::vector<std::string>{"a", "c"}));
	const auto& second = graph.lts.at(first[0].to()).transitions();
	ASSERT_EQ(second.size(), 1);
	EXPECT_EQ(ActionNames(second[0].label().act), (std::vector<std::string>{"b", "d"}));
	EXPECT_EQ(second[0].to().final_condition, Formula{true});

	const auto deadlocked = Compile(Simultaneous{Check{true}, ActionProgram{Action{"a"}}});
	EXPECT_TRUE(deadlocked.lts.at(deadlocked.lts.initial_state()).transitions().empty());
	EXPECT_EQ(deadlocked.lts.initial_state().final_condition, Formula{false});
}

TEST(CharacteristicGraphSemantics, ConditionalAndWhileUseTestSemantics) {
	const Formula condition = Predicate{"ready"};
	const auto conditional = Compile(CgIf{condition, ActionProgram{Action{"a"}}, ActionProgram{Action{"b"}}});
	const auto& choices = conditional.lts.at(conditional.lts.initial_state()).transitions();
	ASSERT_EQ(choices.size(), 2);
	EXPECT_EQ(choices[0].label().condition, condition);
	const Formula negated = UnaryConnective{condition, UnaryKind::Negation};
	EXPECT_EQ(choices[1].label().condition, negated);

	const auto loop = Compile(CgWhile{condition, ActionProgram{Action{"a"}}});
	EXPECT_EQ(loop.lts.initial_state().final_condition, negated);
	ASSERT_EQ(loop.lts.at(loop.lts.initial_state()).transitions().size(), 1);
	EXPECT_EQ(loop.lts.at(loop.lts.initial_state()).transitions()[0].label().condition, condition);
}

TEST(CharacteristicGraphSemantics, RejectsVariablesOutsidePick) {
	EXPECT_THROW(Compile(ActionProgram{Action{"a", {Variable{"x"}}}}), std::invalid_argument);
}

TEST(CharacteristicGraphExport, TikzIsDeterministicAndEscapesText) {
	const auto graph = Compile(ActionProgram{Action{"move_part"}});
	const std::string first = CharacteristicGraphToTikz(graph);
	EXPECT_EQ(first, CharacteristicGraphToTikz(graph));
	EXPECT_NE(first.find("\\begin{tikzpicture}"), std::string::npos);
	EXPECT_NE(first.find("move\\_part"), std::string::npos);
}
