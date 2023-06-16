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
	in(part,4) | machine_drill(part, bit, dmtr, speed, feed, x, y, z, 4) | out(part,4) | nop
*/

inline Resource ExampleResource1() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", { Variable{"part"}, Object{"4"} }};
	scs::Action Out{ "Out", { Variable{"part"}, Object{"4"} }};
	scs::Action MachineDrill{ "MachineDrill", { Variable{"part"}, Variable{"bit"}, Variable{"dmtr"}, Variable{"speed"}, Variable{"feed"}, Variable{"x"},
		Variable{"y"}, Variable{"z"}, Object{"4"} }};

	scs::Branch b1{ scs::ActionProgram{Nop}, scs::ActionProgram{In} };
	scs::Branch b2{ scs::ActionProgram{Out}, scs::ActionProgram{MachineDrill} };


	// Objects and initial valuations
	s0.objects.emplace("4"); // Constant 4


	// Preconditions

	// Successors

	ret.program = std::make_shared<Branch>(b1, b2);
	ret.bat.SetInitial(s0);
	return ret;
}
