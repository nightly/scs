#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

using namespace scs;


/*
#BAT



#Program
loop:
	Nop | In(part, 4); Paint(part, colour, 4); Out(part, 4)

*/

inline Resource ExampleResource4() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop"};
	scs::ActionProgram NopAp{Nop};
	scs::Action In{ "In", { Variable{"part"}, Object{"4"} }};
	scs::ActionProgram InAp{In};	
	scs::Action Out{ "Out", { Variable{"part"}, Object{"4"} }};
	scs::ActionProgram OutAp{Out};
	scs::Action Paint{ "Paint", { Variable{"part"}, Variable{"colour"}, Object{"4"}}};
	scs::ActionProgram PaintAp{Paint};

	Sequence s1(InAp, PaintAp);
	Sequence s2(s1, OutAp);
	Branch nd1(s2, NopAp);
	auto prog = std::make_shared<Loop>(nd1);

	// Objects and initial valuations
	s0.objects.emplace("4"); // Constant 4


	// Preconditions

	// Successors


	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
