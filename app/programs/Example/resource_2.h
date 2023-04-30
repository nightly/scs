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
	nop | (in(part,2); (nop | hold_in_place(part, force,2))*; out(part,2))
*/

inline Resource ExampleResource2() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", {Variable{"part"}, Object{"2"}} };
	scs::Action Out{ "Out", {Variable{"part"}, Object{"2"}} };
	scs::Action HoldInPlace{ "HoldInPlace", {Variable{"part"}, Variable{"force"}, scs::Object{"2"}}};

	scs::ActionProgram ap1{ Nop };
	scs::ActionProgram ap2{ In };
	scs::ActionProgram ap3{ Out };
	scs::ActionProgram ap4{ HoldInPlace };

	Branch nb1{ ap1, ap4 };
	Iteration i1{ nb1 };
	Sequence s1{ ap2, i1 };
	Sequence s2{ s1, ap3 };

	auto prog = std::make_shared<Branch>(ap1, s2);
	ret.program = prog;

	// Objects and initial valuations
	s0.objects.emplace("2"); // Constant 2

	// Preconditions

	// Successors


	ret.bat.initial = s0;
	return ret;
}
