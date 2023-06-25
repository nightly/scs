#pragma once

#include "recipe.h"
#include "resource_1.h"
#include "resource_2.h"
#include "resource_3.h"
#include "resource_4.h"
#include "resource_5.h"

#include "scs/Common/windows.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"


inline static void ExecCharacteristicExample() {

	auto recipe = ExampleRecipe();
	CharacteristicGraph cg_recipe;
	// recipe->AddTransition(cg_recipe, info_recipe);

	CharacteristicGraph cg_resource;
	StateCounter res1_counter;
	StateTracker res1_tracker;
	auto resource = ExampleResource1();
	resource.program->AddTransition(cg_resource, res1_counter, res1_tracker);
	ExportResourceGraph(cg_resource, "resource1");

	// std::cout << *resource.program;
	std::cout << cg_resource;

}

inline static void RunExample() {


}