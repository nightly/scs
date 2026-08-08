#include <gtest/gtest.h>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/Synthesis/Solvers/AStar/a_star.h"
#include "scs/Synthesis/Solvers/GS/gs.h"
#include "scs/Synthesis/Solvers/Core/core.h"
#include "scs/Synthesis/Topology/Complete/complete.h"

using namespace scs;

namespace {

	BasicActionTheory TestBat() {
		BasicActionTheory bat;
		bat.is_global = true;
		bat.objects = {Object{"a"}, Object{"b"}};
		bat.pre.emplace("Nop", Poss{true});
		bat.pre.emplace("Done", Poss{{Variable{"item"}}, true});
		bat.pre.emplace("Aux", Poss{{Variable{"item"}}, true});
		bat.pre.emplace("Pair", Poss{{Variable{"left"}, Variable{"right"}}, true});
		bat.types.emplace("Nop", ActionType::Nop);
		bat.types.emplace("Done", ActionType::Manufacturing);
		bat.types.emplace("Aux", ActionType::Preparatory);
		bat.types.emplace("Pair", ActionType::Preparatory);
		bat.SetInitial(Situation{});
		bat.SetCoopMx(CoopMatrix{1});
		bat.SetRoutesMx(RoutesMatrix{1});
		return bat;
	}

	CharacteristicGraph Compile(const IProgram& program, ProgramType type) {
		return CharacteristicGraph{program.clone(), type};
	}

	std::vector<CompoundAction> PlanActions(const Candidate& candidate) {
		std::vector<CompoundAction> result;
		for (const auto& [state, outgoing] : candidate.plan.lts.states()) {
			(void)state;
			for (const auto& transition : outgoing.transitions()) {
				result.emplace_back(transition.label().act);
			}
		}
		return result;
	}

	void ExpectBindingController(const SynthesisReport& report) {
		ASSERT_EQ(report.status, SynthesisStatus::Solved);
		ASSERT_TRUE(report.candidate.has_value());
		EXPECT_EQ(report.candidate->total_transitions, 4);

		const auto actions = PlanActions(*report.candidate);
		size_t nops = 0;
		size_t completions = 0;
		std::unordered_set<Object> completed_objects;
		for (const auto& action : actions) {
			if (action.Actions().size() == 1 && action.Actions()[0].name == "Nop") {
				++nops;
				continue;
			}
			ASSERT_EQ(action.Actions().size(), 2);
			EXPECT_EQ(action.Actions()[0].name, "Done");
			EXPECT_EQ(action.Actions()[1].name, "Aux");
			const auto& done_object = std::get<Object>(action.Actions()[0].terms[0]);
			EXPECT_EQ(done_object, std::get<Object>(action.Actions()[1].terms[0]));
			completed_objects.emplace(done_object);
			++completions;
		}
		EXPECT_EQ(nops, 2);
		EXPECT_EQ(completions, 2);
		EXPECT_EQ(completed_objects, (std::unordered_set<Object>{Object{"a"}, Object{"b"}}));
	}

}

TEST(SolverBindings, PickProductsAllowRepetitionAndGuardsFilterChoices) {
	auto bat = TestBat();
	const Variable left{"left"};
	const Variable right{"right"};
	const ObjectSet objects{Object{"a"}, Object{"b"}};
	const CgTransition unrestricted{Action{"Pair", {left, right}}, {left, right}, true};

	const auto repeated = InstantiateTransition(unrestricted, {}, bat.Initial(), bat, objects);
	ASSERT_EQ(repeated.size(), 4);
	EXPECT_TRUE(std::ranges::any_of(repeated, [](const auto& step) {
		const auto& terms = step.transition.act.Actions()[0].terms;
		return std::get<Object>(terms[0]) == std::get<Object>(terms[1]);
	}));

	const Formula different = BinaryConnective{left, right, BinaryKind::NotEqual};
	const CgTransition restricted{Action{"Pair", {left, right}}, {left, right}, different};
	const auto distinct = InstantiateTransition(restricted, {}, bat.Initial(), bat, objects);
	ASSERT_EQ(distinct.size(), 2);
	for (const auto& step : distinct) {
		const auto& terms = step.transition.act.Actions()[0].terms;
		EXPECT_NE(std::get<Object>(terms[0]), std::get<Object>(terms[1]));
	}
}

TEST(SolverBindings, AStarAndGreedyPreserveBindingsAcrossNopAndCompleteCompoundActions) {
	auto bat = TestBat();
	const Variable recipe_item{"recipe_item"};
	const Variable resource_item{"resource_item"};
	const Pick recipe{{recipe_item}, ActionProgram{Action{"Done", {recipe_item}}}};
	const CompoundAction completion{std::vector<Action>{
		Action{"Done", {resource_item}},
		Action{"Aux", {resource_item}},
	}};
	const Pick resource{{resource_item}, Sequence{
		ActionProgram{Action{"Nop"}},
		ActionProgram{completion},
	}};

	auto recipe_graph = Compile(recipe, ProgramType::Recipe);
	std::vector<CharacteristicGraph> resource_graphs;
	resource_graphs.emplace_back(Compile(resource, ProgramType::Resource));
	CompleteTopology topology{&resource_graphs};

	const auto& initial_topology_edges = topology.at(topology.initial_state()).transitions();
	ASSERT_EQ(initial_topology_edges.size(), 1);
	ASSERT_EQ(initial_topology_edges[0].label().components.size(), 1);
	EXPECT_EQ(initial_topology_edges[0].label().components[0].label().act.Actions()[0].name, "Nop");

	Limits limits;
	limits.global_transition_limit = 8;
	limits.stage_transition_limit = 3;
	AStar astar{resource_graphs, recipe_graph, bat, topology, limits};
	ExpectBindingController(astar.Synthesise());

	GS greedy{resource_graphs, recipe_graph, bat, topology, limits, false, std::mt19937{7}};
	ExpectBindingController(greedy.Synthesise());
}

TEST(SolverBindings, SemanticallyFinalRecipeMayTerminateDespiteOutgoingEdges) {
	auto bat = TestBat();
	const Iteration recipe{ActionProgram{Action{"Done", {Object{"a"}}}}};
	auto recipe_graph = Compile(recipe, ProgramType::Recipe);
	std::vector<CharacteristicGraph> resource_graphs;
	resource_graphs.emplace_back(Compile(Nil{}, ProgramType::Resource));
	CompleteTopology topology{&resource_graphs};

	ASSERT_FALSE(recipe_graph.lts.at(recipe_graph.lts.initial_state()).transitions().empty());
	ASSERT_TRUE(topology.at(topology.initial_state()).transitions().empty());

	AStar astar{resource_graphs, recipe_graph, bat, topology};
	const auto astar_report = astar.Synthesise();
	ASSERT_EQ(astar_report.status, SynthesisStatus::Solved);
	ASSERT_TRUE(astar_report.candidate.has_value());
	EXPECT_EQ(astar_report.candidate->total_transitions, 0);

	GS greedy{resource_graphs, recipe_graph, bat, topology, Limits{}, false, std::mt19937{7}};
	const auto greedy_report = greedy.Synthesise();
	ASSERT_EQ(greedy_report.status, SynthesisStatus::Solved);
	ASSERT_TRUE(greedy_report.candidate.has_value());
	EXPECT_EQ(greedy_report.candidate->total_transitions, 0);
}
