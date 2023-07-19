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
	Nop | (In(part, 1); (Nop | Clamp(part, force, 1))∗ ; Out(part, 1)
*/

inline Resource ExampleResource1() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Load{"part", {Object{"1"} }};
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
	scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
	scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};

	Branch nd1(ActionProgram{Nop}, ActionProgram{Clamp});
	Loop l1(nd1);
	Sequence s1(ActionProgram{In}, l1);
	Sequence s2(s1, ActionProgram{Out});

	Branch nd2(ActionProgram{Nop}, s2);

	// Objects and initial valuations
	s0.objects.emplace("1"); // Constant 1


	// Preconditions

	// Successors

	ret.program = std::make_shared<Loop>(nd2);
	ret.bat.SetInitial(s0);
	return ret;
}
