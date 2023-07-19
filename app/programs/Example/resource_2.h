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
	Load(part, 2) | Nop∗ | Store(part, status, 2) | In(part, 2) | Out(part, 2)
*/

inline Resource ExampleResource2() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Load{ "Load", { Variable{"part"}, scs::Object{"2"} }};
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", {Variable{"part"}, Object{"2"}} };
	scs::Action Out{ "Out", {Variable{"part"}, Object{"2"}} };
	scs::Action Store{ "Load", { Variable{"part"}, Variable{"status"}, scs::Object{"2"}}};

	scs::Loop l1(ActionProgram{Nop}); // Nop*

	scs::Branch nd1(ActionProgram{Load}, l1); // Load | Nop*
	scs::Branch nd2(nd1, ActionProgram{Store});
	Branch nd3(nd2, ActionProgram{In});
	Branch nd4(nd3, ActionProgram{Out});

	// Objects and initial valuations
	s0.objects.emplace("2"); // Constant 2

	// Preconditions

	// Successors

	ret.program = std::make_shared<Loop>(nd4);
	ret.bat.SetInitial(s0);
	return ret;
}
