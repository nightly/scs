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
	auto common = ExampleCommon();
	auto common_bat = ExampleCommonBAT();

	auto resource1 = ExampleResource1();
	CharacteristicGraph cg_resource1(resource1.program, ProgramType::Resource);
	ExportGraph(cg_resource1, "resource1");

	auto resource2 = ExampleResource2();
	CharacteristicGraph cg_resource2(resource2.program, ProgramType::Resource);
	ExportGraph(cg_resource2, "resource2");

	auto resource3 = ExampleResource3();
	CharacteristicGraph cg_resource3(resource3.program, ProgramType::Resource);
	ExportGraph(cg_resource3, "resource3");

	auto resource4 = ExampleResource4();
	CharacteristicGraph cg_resource4(resource4.program, ProgramType::Resource);
	ExportGraph(cg_resource4, "resource4");

	auto recipe_prog = ExampleRecipe();
	CharacteristicGraph cg_recipe(recipe_prog, ProgramType::Recipe);
	ExportGraph(cg_recipe, "recipe");

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
}