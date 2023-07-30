#include <iostream>
#include <format>

#include "programs/programs.h"
#include "Hinge/run.h"
#include "HingeQuick/run.h"
// #include "scs/Memory/tracker.h"
#include "scs/Common/timer.h"

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

using namespace scs;

int main(int argc, const char* argv[]) {
	Timer t1("Total time");
	scs::SetConsoleEncoding();
	// LogModeInfo();

	// scs::examples::RunHinge(); // modify Limits b4 running this
	scs::examples::RunHingeQuick(); // 10m
}
