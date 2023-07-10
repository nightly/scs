﻿#pragma once

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
	Nop | (In(part, 1); (Nop | Clamp(part, force, 1)))∗ ; Out(part, 1)
*/

inline Resource ExampleResource1() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Load{"part", {Object{"1"} }};
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
	scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
	scs::Action MachineDrill{ "MachineDrill", { Variable{"part"}, Variable{"bit"}, Variable{"dmtr"}, Variable{"speed"}, Variable{"feed"}, Variable{"x"},
		Variable{"y"}, Variable{"z"}, Object{"1"} }};

	scs::Branch b1{ scs::ActionProgram{Nop}, scs::ActionProgram{In} };
	scs::Branch b2{ scs::ActionProgram{Out}, scs::ActionProgram{MachineDrill} };
	scs::Branch b3(b1, b2);
	scs::Branch b4(scs::ActionProgram{Load}, b3);

	// Objects and initial valuations
	s0.objects.emplace("1"); // Constant 1


	// Preconditions

	// Successors

	ret.program = std::make_shared<Loop>(b4);
	ret.bat.SetInitial(s0);
	return ret;
}
