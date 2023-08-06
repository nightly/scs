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

	Timer t1("Total time");
	scs::SetConsoleEncoding();
	// LogModeTrace();

	ExecutionType type = ExecutionType::GBFS;
	size_t example = 1;
	bool just_export = false;

	if (example == 1) {
		scs::examples::RunHingeQuick(type, just_export);
	} else if (example == 2) {
		scs::examples::RunHinge(type, just_export);
	} else if (example == 3) {
		scs::examples::RunHingeExtended(type, just_export);
	}
}
