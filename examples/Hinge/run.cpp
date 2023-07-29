#pragma once

#include "recipe.h"
#include "resource_1.h"
#include "resource_2.h"
#include "resource_3.h"
#include "resource_4.h"
#include "common.h"

#include "scs/Common/windows.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/synthesis.h"

namespace scs::examples {

	void RunHinge() {
		// ------- Load BATs, Cg --------
		std::vector<CharacteristicGraph> graphs;
		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();

		auto resource1 = HingeResource1();
		graphs.emplace_back(resource1.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "resource1");

		auto resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "resource2");	
		
		auto resource3 = HingeResource3();
		graphs.emplace_back(resource3.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "resource3");		
		
		auto resource4 = HingeResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);
		ExportGraph(graphs.back(), "resource4");

		auto recipe_prog = HingeRecipe();
		CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);
		ExportGraph(graph_recipe, "recipe");

		// ------------------------------

		// ----- Coop & Routes -----
		CoopMatrix cm(4);
		cm.Add(0, 1);
		RoutesMatrix rm(4);
		rm.Add(0, 1);
		rm.Add(0, 2);
		rm.Add(0, 3);
		rm.Add(1, 2);
		rm.Add(1, 3);
		rm.Add(2, 3);
		// -------------------------

		// ------ Global BAT -------
		std::vector<scs::BasicActionTheory> bats{resource1.bat, resource2.bat, resource3.bat, resource4.bat, common_bat};
		auto global = CombineBATs(bats, cm, rm);
		// -------------------------

		// std::cout << common.within_reach;
		// std::cout << global.successors["part"].Form() << std::endl;
		// std::cout << global.pre["In"].Form() << std::endl;

		// std::cout << global;
		// std::cout << global.Initial();
		// std::cout << *graphs[3].lts.states().at(2).key_;
		// -------------------------------

		Limits lim;
		CompleteTopology topology(&graphs, false);
		ExportTopology(topology);

		Best best(graphs, graph_recipe, global, topology, lim);
		auto controller = best.Synthethise();

		GenerateImagesFromDot();
	}
}