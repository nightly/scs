#include <iostream>

#include "programs/programs.h"
#include "Hinge/run.h"
#include "HingeQuick/run.h"
#include "scs/Common/timer.h"
#include "scs/Common/windows.h"

using namespace scs;
using namespace scs::examples;

int main(int argc, const char* argv[]) {

	Timer t1("Total time");
	scs::SetConsoleEncoding();
	// LogModeTrace();

	ExecutionType type = ExecutionType::AStar;
	size_t example = 1;

	if (example == 1) {
		scs::examples::RunHingeQuick(type);
	} else if (example == 2) {
		scs::examples::RunHinge(type);
	}
}
