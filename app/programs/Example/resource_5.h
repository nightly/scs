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
	if entered(5) then
		in(part,5) | spray_glue(part, glue_type, 5) | operate_machine(j,5) |
		in_cell(part, ... , 5) | out(part,5) | (exit(5) ; safety_switch(off,5)) | nop
	else 
		(safety_switch(on,5); enter(5)) | nop
	endIf
*/

inline Resource ExampleResource5() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop", {} };


	// Objects and initial valuations
	s0.objects.emplace("5"); // Constant 5


	// Preconditions

	// Successors

	
	ret.bat.initial = s0;
	return ret;
}
