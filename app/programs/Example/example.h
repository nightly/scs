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

	auto resource = ExampleResource1();
	CharacteristicGraph cg_resource(resource.program);
	ExportResourceGraph(cg_resource, "resource1");

	// std::cout << *resource.program;
	std::cout << cg_resource;

}

inline static void RunExample() {


}