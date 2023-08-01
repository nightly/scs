#include <iostream>

#include "programs/programs.h"
#include "Hinge/run.h"
#include "HingeQuick/run.h"
#include "scs/Common/timer.h"

using namespace scs;

int main(int argc, const char* argv[]) {
	Timer t1("Total time");
	scs::SetConsoleEncoding();
	// LogModeInfo();

	// scs::examples::RunHinge();
	scs::examples::RunHingeQuick();
}
