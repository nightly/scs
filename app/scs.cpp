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
	FluentProg_();


	// s_allocation_metrics.PrintUsage();
	
	Plan plan;
	Plan plan2;
	plan.branches.emplace_back(plan2, DeviationType::Iteration);
	return 0;
}