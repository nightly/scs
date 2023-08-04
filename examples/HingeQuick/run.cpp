#pragma once

#include "recipe.h"
#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"
#include "Hinge/common.h"

#include "scs/Common/timer.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/synthesis.h"

namespace scs::examples {

	void RunHingeQuick() {
		// ------- Load BATs, Cg --------
		std::vector<CharacteristicGraph> graphs;
		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();

		auto resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "Hinge/Quick/Resource2");

		auto resource4 = HingeResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "Hinge/Quick/Resource4");

		auto recipe_prog = HingeRecipeQuick();
		CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);
		ExportGraph(graph_recipe, "Hinge/Quick/Recipe");

		// ------------------------------

		// ----- Coop & Routes -----
		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(2, 4);

		// -------------------------

		// ------ Global BAT -------
		std::vector<scs::BasicActionTheory> bats{common_bat, 
			resource2.bat, resource4.bat};
		auto global = CombineBATs(bats, cm, rm);
		// -------------------------

		// std::cout << common.within_reach;
		// std::cout << global.successors["part"].Form() << std::endl;
		// std::cout << global.pre["In"].Form() << std::endl;

		// std::cout << global;
		// std::cout << global.Initial();
		// std::cout << *graphs[3].lts.states().at(2).key_;
		// -------------------------------

		Limits lim{ .global_transition_limit = 10, .global_cost_limit = 200,
			.stage_transition_limit = 3, .stage_cost_limit = 50, .fairness_limit = 20 };

		Timer topology_timer("Topology time");
		CompleteTopology topology(&graphs, false);
		topology_timer.StopWithWrite();

		Best best(graphs, graph_recipe, global, topology, lim);

		if (1) {
			auto controller = best.Synthethise();
			ExportController(controller.value().plan, "Hinge/Quick/Controller");
		}

		ExportTopology(topology, "Hinge/Quick/Topology");
		GenerateImagesFromDot("../../exports/Hinge/Quick/");
	}
}