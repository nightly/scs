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
	scs::ActionProgram LoadAp(Load);

	scs::Action Nop{ "Nop", {} };
	scs::ActionProgram NopAp(Nop);

	scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
	scs::ActionProgram InAp(In);

	scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
	scs::ActionProgram OutAp(Out);

	scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
	scs::ActionProgram ClampAp(Clamp);

	scs::Branch nd1(NopAp, ClampAp);
	scs::Loop l1(nd1);
	Sequence s1(InAp, l1);
	Sequence s2(s1, OutAp);

	scs::Branch nd2(NopAp, s2);

	// Objects and initial valuations
	s0.objects.emplace("1"); // Constant 1


	// Preconditions

	// Successors

	ret.program = std::make_shared<Loop>(nd2);
	ret.bat.SetInitial(s0);
	return ret;
}
