#include <iostream>
#include <format>

#include "programs/programs.h"
// #include "scs/Memory/tracker.h"

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Synthesis/synthesis.h"

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

using namespace scs;

int main(int argc, const char* argv[]) {	
	scs::SetConsoleEncoding();

	RunExample();
}
