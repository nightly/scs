#pragma once

#include "recipe.h"
#include "resource_1.h"
#include "resource_2.h"
#include "resource_3.h"
#include "resource_4.h"
#include "resource_5.h"

#include "scs/Common/windows.h"


inline static void ExecTracesExample() {

	auto recipe = ExampleRecipe();

	auto resource = ExampleResource1();
	// std::cout << *resource.program;

	Execution exec;
	resource.program->Decompose(exec);
	std::cout << exec;
}

inline static void RunExample() {


}