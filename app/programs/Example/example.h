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

inline static void RunExample() {
	// ------- Load BATs, Cg --------
	std::vector<CharacteristicGraph> graphs;
	auto common = ExampleCommon();
	auto common_bat = ExampleCommonBAT();

	auto resource1 = ExampleResource1();
	graphs.emplace_back(resource1.program, ProgramType::Resource);
	ExportGraph(graphs[0], "resource1");

	auto resource2 = ExampleResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);
	ExportGraph(graphs[1], "resource2");

	auto resource3 = ExampleResource3();
	graphs.emplace_back(resource3.program, ProgramType::Resource);
	ExportGraph(graphs[2], "resource3");

	auto resource4 = ExampleResource4();
	graphs.emplace_back(resource4.program, ProgramType::Resource);
	ExportGraph(graphs[3], "resource4");

	auto recipe_prog = ExampleRecipe();
	CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);
	ExportGraph(graph_recipe, "recipe");

	DotOutput();
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

	std::cout << global;
	// std::cout << global.Initial();
	std::cout << *graphs[3].lts.states().at(2).key_;
	// -------------------------------

	Limits lim;
	Best best(graphs, graph_recipe, global, lim);
	auto controller = best.Synthethise();
}