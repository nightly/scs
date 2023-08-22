#include <iostream>

#include "programs/programs.h"
#include "Hinge/Full/run.h"
#include "Hinge/Quick/run.h"
#include "Hinge/Extended/run.h"
#include "scs/Common/timer.h"
#include "scs/Common/windows.h"

using namespace scs;
using namespace scs::examples;

int main(int argc, const char* argv[]) {
	Timer t1("Total");
	scs::SetConsoleEncoding();
	// LogModeTrace();

	ExecutionType type = ExecutionType::GS;
	size_t example = 2;
	size_t recipe_variant = 3;
	bool just_export = false;

	bool shuffling = true;
	std::mt19937 rng(std::random_device{}());

	if (example == 1) {
		scs::examples::RunHingeQuick(type, recipe_variant, just_export, shuffling, rng);
	} else if (example == 2) {
		scs::examples::RunHinge(type, recipe_variant, just_export, shuffling, rng);
	} else if (example == 3) {
		scs::examples::RunHingeExtended(type, recipe_variant, just_export, shuffling, rng);
	}
}
