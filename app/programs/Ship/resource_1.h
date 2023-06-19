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
ship(var(x)) | move(var(x), var(loc)) | arrive(var(x))
*/

inline Resource ShipResource1() {
	Resource ret;
	Situation s0;

	// Objects and initial valuations
	s0.objects.emplace("shipyard");
	scs::RelationalFluent is_loc_rf = { "isLoc" };
	is_loc_rf.AddValuation({ scs::Object{"shipyard"}}, true );
	s0.AddFluent(is_loc_rf);
	ret.bat.SetInitial(s0);

	// Preconditions

	// Successors

	// Program

	ActionProgram ap1{ {"Ship", {scs::Variable{"x"} }}};
	ActionProgram ap2{ {"Move", {scs::Variable{"x"}, scs::Variable{"l"} }}};
	ActionProgram ap3{ {"Arrive", {scs::Variable{"x"} }}};

	ActionProgram ap4{ {"Throw", {scs::Variable{"x"} }}};
	ActionProgram ap5{ {"Discard", {scs::Variable{"x"} }}};
	Sequence s1{ ap4, ap5 };


	Branch nd1{ ap1, ap2 };
	Branch nd2{ nd1, ap3 };
	Branch nd3{ nd2, s1 }; // @Todo
	auto prog = std::make_shared<Branch>(nd2);
	ret.program = prog;

	return ret;
}