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
	in(part,3)
	| (equip(pressure_hollow,3) | equip(pressure_flat,3));
	nop* ; pressure(part, force, type,3)* ; unequip(3)
	| equip(gripper,3) ; position(part1, part2, part, x, y, z, 3)
	| in_cell(part, weight, material, dimx, dimy, dimz, 3)
	| out_cell(part, code,3)
	| nop
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
