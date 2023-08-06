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
	size_t example = 2;

	if (example == 1) {
		scs::examples::RunHingeQuick(type);
	} else if (example == 2) {
		scs::examples::RunHinge(type);
	} else if (example == 3) {
		scs::examples::RunHingeExtended(type);
	}
}
