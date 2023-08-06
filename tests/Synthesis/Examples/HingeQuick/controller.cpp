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
		Best best(graphs, graph_recipe, global, topology, lim);

		auto controller = best.Synthethise();
		controller_val = *controller;
	}

	// void TearDown() override {}
};

TEST_F(HingeQuickTestController, Num) {
	EXPECT_EQ(controller_val.total_transitions, 8);
	EXPECT_EQ(controller_val.total_cost, 22);
}