#include <iostream>
#include <format>

#include "programs/programs.h"
// #include "scs/Memory/tracker.h"

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Synthesis/synthesis.h"

using namespace scs;

int main(int argc, const char* argv[]) {
	auto recipe = ShipRecipe();
	// std::cout << *s << std::endl;
	// std::cout << "=================================================" << std::endl;

	auto resource = ExampleResource4();
	// std::cout << *s2.program;

	auto dec = resource.program->Decompose(resource.bat.Initial());
	// std::cout << dec;

	return 0;
}

// FluentProg_();
// s_allocation_metrics.PrintUsage();
