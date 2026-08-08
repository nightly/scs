#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#BAT

#Program
loop:
	Nop | In(part, 4); Paint(part, colour, 4); Out(part, 4)

*/

namespace scs::examples {

	inline Resource HingeResource4() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Nop{ "Nop"};
		scs::Action In{ "In", { Variable{"part"}, Object{"4"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"4"} }};
		scs::Action Paint{ "Paint", { Variable{"part"}, Variable{"colour"}, Object{"4"} }};

		Pick paint({Variable{"colour"}}, ActionProgram{Paint});
		Sequence s1(ActionProgram{ In }, paint);
		Sequence s2(s1, ActionProgram{ Out });
		Pick handle_part({Variable{"part"}}, s2);
		Branch nd1(handle_part, ActionProgram{ Nop });
		auto prog = std::make_shared<Loop>(nd1);

		ret.bat = ParseBasicActionTheory(R"(
objects 4, metallic_red, metallic_blue
type Paint = manufacturing

poss Paint(part, colour, i) = at(part, i) and pigment(colour)
)");

		ret.program = prog;
		return ret;
	}

}
