#include <gtest/gtest.h>

#include "Hinge/Quick/recipe.h"
#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"
#include "Hinge/common.h"

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/synthesis.h"
#include "scs/Common/windows.h"

using namespace scs::examples;
using namespace scs;

class HingeQuickTestController : public ::testing::Test {
protected:
	HingeCommon common;
	scs::Resource resource2, resource4;
	scs::BasicActionTheory global, common_bat;
	scs::CoopMatrix cm{10};
	scs::RoutesMatrix rm{10};
	Candidate controller_val;
	Candidate legacy_controller_val;
	Candidate greedy_controller_val;
	size_t cached_situations = 0;
	size_t legacy_cached_situations = 0;
	size_t greedy_cached_situations = 0;
	SynthesisReport astar_report;
	SynthesisReport greedy_report;
	SynthesisReport cancelled_report;
	size_t best_callback_count = 0;

	void SetUp() override {
		SetConsoleEncoding();
		
		std::vector<CharacteristicGraph> graphs;
		common = HingeCommon();
		common_bat = HingeCommonBAT();

		resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);

		resource4 = HingeResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);

		auto recipe_prog = HingeRecipeQuick();
		CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);

		// ----- Coop & Routes -----
		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(2, 4);
		// ------------------------

		std::vector<scs::BasicActionTheory> bats{common_bat,
			resource2.bat, resource4.bat};
		auto global = CombineBATs(bats, cm, rm);

		CompleteTopology topology(&graphs, false);

		Limits lim;
		AStar best(graphs, graph_recipe, global, topology, lim);

		SearchControl best_control;
		best_control.on_best_candidate = [this](const Candidate& candidate, const SynthesisStatistics& statistics) {
			EXPECT_EQ(candidate.total_cost, 22);
			EXPECT_GT(statistics.visited_situations, 0);
			++best_callback_count;
		};
		astar_report = best.Synthesise(best_control);
		auto controller = astar_report.candidate;
		ASSERT_TRUE(controller.has_value());
		controller_val = *controller;
		cached_situations = best.cache_.SizeSituationStates();

		AStar legacy(graphs, graph_recipe, global, topology, lim, false);
		auto legacy_controller = legacy.Synthethise();
		ASSERT_TRUE(legacy_controller.has_value());
		legacy_controller_val = *legacy_controller;
		legacy_cached_situations = legacy.cache_.SizeSituationStates();

		Limits greedy_limits{ .global_transition_limit = 50, .global_cost_limit = 200,
			.stage_transition_limit = 4, .stage_cost_limit = 50, .fairness_limit = 20 };
		GS greedy(graphs, graph_recipe, global, topology, greedy_limits, false, std::mt19937{2010});
		greedy_report = greedy.Synthesise();
		auto greedy_controller = greedy_report.candidate;
		ASSERT_TRUE(greedy_controller.has_value());
		greedy_controller_val = *greedy_controller;
		greedy_cached_situations = greedy.cache_.SizeSituationStates();

		AStar cancelled(graphs, graph_recipe, global, topology, lim);
		SearchControl control;
		control.deadline = std::chrono::steady_clock::now();
		cancelled_report = cancelled.Synthesise(control);
	}

	// void TearDown() override {}
};

TEST_F(HingeQuickTestController, Num) {
	EXPECT_EQ(controller_val.total_transitions, 8);
	EXPECT_EQ(controller_val.total_cost, 22);
	EXPECT_GT(cached_situations, 0);
	EXPECT_EQ(legacy_controller_val.total_transitions, controller_val.total_transitions);
	EXPECT_EQ(legacy_controller_val.total_cost, controller_val.total_cost);
	EXPECT_EQ(legacy_cached_situations, 0);
	EXPECT_GT(greedy_controller_val.total_transitions, 0);
	EXPECT_GT(greedy_cached_situations, 0);
	EXPECT_EQ(astar_report.status, SynthesisStatus::Solved);
	EXPECT_EQ(astar_report.statistics.visited_situations, 19);
	EXPECT_EQ(best_callback_count, 1);
	EXPECT_EQ(greedy_report.status, SynthesisStatus::Solved);
	EXPECT_EQ(greedy_report.statistics.visited_situations, 12);
	EXPECT_EQ(cancelled_report.status, SynthesisStatus::Cancelled);
	EXPECT_FALSE(cancelled_report.candidate.has_value());
}
