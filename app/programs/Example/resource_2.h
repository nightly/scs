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

	scs::ActionProgram NopAp{ Nop };
	scs::ActionProgram InAp{ In };
	scs::ActionProgram OutAp{ Out };
	scs::ActionProgram HoldInPlaceAp{ HoldInPlace };


	Branch nb1{ NopAp, HoldInPlaceAp }; // Nop || HoldInPlace
	Iteration i1{ nb1 }; // Iteration of (Nop | HoldInPlace)
	Sequence s1{ InAp, i1 }; // In(part, 2); iteration 
	Sequence s2{ s1, OutAp }; // Iteration; Out(part, 2)

	Branch nb_last{ NopAp, s2 };

	auto prog = std::make_shared<Loop>(nb_last);
	ret.program = prog;

	// Objects and initial valuations
	s0.objects.emplace("2"); // Constant 2

	// Preconditions

	// Successors


	ret.bat.SetInitial(s0);
	return ret;
}
