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
	scs::Action In{ "In", { Variable{"part"}, Object{"4"} }};
	scs::Action Out{ "Out", { Variable{"part"}, Object{"4"} }};
	scs::Action Paint{ "Paint", { Variable{"part"}, Variable{"colour"}, Object{"4"}}};

	Sequence s1(ActionProgram{In}, ActionProgram{Paint});
	Sequence s2(s1, ActionProgram{Out});
	Branch nd1(s2, ActionProgram{Nop});
	auto prog = std::make_shared<Loop>(nd1);

	// Objects and initial valuations
	s0.objects.emplace("4"); // Constant 4


	// Preconditions
	ret.bat.pre["Paint"] = { {scs::Variable{"part"}, scs::Variable{"colour"}, scs::Variable{"i"}}, 
		Predicate{"at", {Variable{"part"}, Variable{"i"}}}};
	// Successors


	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
