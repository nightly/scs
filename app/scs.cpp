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
	auto s = ShipRecipe();
	std::cout << *s << std::endl;
	std::cout << "=================================================" << std::endl;

	auto s2 = ShipResource1();
	// std::cout << *s2.program;

	return 0;
}

// FluentProg_();
// s_allocation_metrics.PrintUsage();
