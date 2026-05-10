#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"
#include "Hinge/common.h"

/*
#BAT

#Program
loop:
	Load(part, 2) | Nop∗ | Out(part, 2)
*/

namespace scs::examples {

	inline Resource HingeGroundedResource2() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Load{ "Load", { Variable{"part"}, scs::Object{"2"} }};
		scs::Action Nop{ "Nop", {} };
		scs::Action Out{ "Out", { Variable{"part"}, Object{"2"} } };

		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"2"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"2"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram LoadG1(Action{"Load", { Object{"brass"}, Object{"2"}}});
		ActionProgram LoadG2(Action{"Load", { Object{"tube"}, Object{"2"}}});
		Branch LoadB(LoadG1, LoadG2);

		scs::Loop l1(ActionProgram{ Nop }); // Nop*
		scs::Branch nd1(LoadB, l1); // Load | Nop*
		Branch nd2(nd1, OutB);

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
