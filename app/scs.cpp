#include <iostream>
#include <format>

#include "programs/programs.h"
#include "Hinge/run.h"
// #include "scs/Memory/tracker.h"

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

using namespace scs;

int main(int argc, const char* argv[]) {	
	scs::SetConsoleEncoding();
	// LogModeInfo();

	scs::examples::RunHinge();
}
