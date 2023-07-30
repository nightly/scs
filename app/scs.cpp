#include <iostream>
#include <format>

#include "programs/programs.h"
#include "Hinge/run.h"
// #include "scs/Memory/tracker.h"
#include "scs/Common/timer.h"

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

using namespace scs;

int main(int argc, const char* argv[]) {
	Timer t1("timer");
	scs::SetConsoleEncoding();
	// LogModeInfo();

	scs::examples::RunHinge();
}
