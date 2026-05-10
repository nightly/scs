#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"
#include "common.h"

/*
#BAT

#Program
loop:
	Load(part, 2) | Nop∗ | Out(part, 2)
*/

namespace scs::examples {

	inline Resource HingeResource2() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Load{ "Load", { Variable{"part"}, scs::Object{"2"} }};
		scs::Action Nop{ "Nop", {} };
		scs::Action Out{ "Out", { Variable{"part"}, Object{"2"} } };

		scs::Loop l1(ActionProgram{ Nop }); // Nop*
		scs::Branch nd1(ActionProgram{ Load }, l1); // Load | Nop*
		Branch nd2(nd1, ActionProgram{ Out });

		ret.bat = ParseBasicActionTheory(R"(
objects 2
type Load = manufacturing

poss Load(part, i) = part(part) and on_site(part)

ssa on_site(part) = cv and not (exists i. a = Load(part, i)) and not (exists s, i. a = Store(part, s, i))
)");

		//////////////
		ret.program = std::make_shared<Loop>(nd2);
		return ret;
	}

}
