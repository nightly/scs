#pragma once

#include "recipe.h"
#include "resource_1.h"
#include "resource_2.h"
#include "resource_3.h"
#include "resource_4.h"
#include "common.h"

#include "scs/Common/timer.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/synthesis.h"
#include "scs/Common/windows.h"

namespace scs::examples {

	void RunHinge() {
		// ------- Load BATs, Cg --------
		std::vector<CharacteristicGraph> graphs;
		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();

		auto resource1 = HingeResource1();
		graphs.emplace_back(resource1.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "Hinge/Full/Resource1");

		auto resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "Hinge/Full/Resource2");

		auto resource3 = HingeResource3();
		graphs.emplace_back(resource3.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "Hinge/Full/Resource3");

		auto recipe_prog = HingeRecipe();
		CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);
		ExportGraph(graph_recipe, "Hinge/Full/Recipe");

		// ------------------------------

		// ----- Coop & Routes -----
		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(1, 2);

		// -------------------------

		// ------ Global BAT -------
		std::vector<scs::BasicActionTheory> bats{common_bat, resource1.bat, resource2.bat, resource3.bat};
		auto global = CombineBATs(bats, cm, rm);
		// -------------------------

		// std::cout << common.within_reach;
		// std::cout << global.successors["part"].Form() << std::endl;
		// std::cout << global.pre["In"].Form() << std::endl;

		// std::cout << global;
		// std::cout << global.Initial();
		// std::cout << *graphs[3].lts.states().at(2).key_;
		// -------------------------------

		Limits lim { .global_transition_limit = 50, .global_cost_limit = 200,  
			.stage_transition_limit = 4, .stage_cost_limit = 50, .fairness_limit = 20};
		
		Timer topology_timer("Topology time");
		CompleteTopology topology(&graphs, false);
		topology_timer.StopWithWrite();

		Best best(graphs, graph_recipe, global, topology, lim);

		if (1) {
			auto controller = best.Synthethise();
			ExportController(controller.value().plan, "Hinge/Full/Controller");
		}


		ExportTopology(topology, "Hinge/Full/Topology");
		GenerateImagesFromDot("../../exports/Hinge/Full/");
	}
}