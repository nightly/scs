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
	if equipped(bit, 3) then
		RadialDrill(part, radius, bit, 3)
	else
		AttachBit(3mm, 3) | AttachBit(5mm, 3)
	endIf |
	In(part, 4) | Out(part, 4)
	| ApplyAdhesive(part, type, 3) | Paint(part, colour, 3)
*/

inline Resource ExampleResource3() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop"};
	scs::ActionProgram NopAp{Nop};



	// Objects and initial valuations
	s0.objects.emplace("3"); // Constant 3


	// Preconditions

	// Successors



	auto prog = std::make_shared<Loop>(NopAp);
	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
