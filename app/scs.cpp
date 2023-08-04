#include <iostream>

#include "programs/programs.h"
#include "Hinge/run.h"
#include "HingeQuick/run.h"
#include "scs/Common/timer.h"

using namespace scs;

int main(int argc, const char* argv[]) {

	Timer t1("Total time");
	scs::SetConsoleEncoding();
	// LogModeTrace();

	size_t example = 1;

	if (example == 1) {
		scs::examples::RunHingeQuick();
	} else if (example == 2) {
		scs::examples::RunHinge();
	}

}
