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
	In(part, 3) | Out(part, 3)
	| ApplyAdhesive(part, type, 3)
*/

inline Resource ExampleResource3() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop"};
	scs::Action RadialDrill{"RadialDrill", { Variable{"part"}, Variable{"radius"}, Variable{"bit"}, Object{"3"} }};
	scs::Action AttachBit3mm{"AttachBit", { Object{"3mm"}, Object{"3"} }};
	scs::Action AttachBit5mm{"AttachBit", { Object{"5mm"}, Object{"3"} }};
	scs::Action In{"In", { Variable{"part"}, Object{"3"} }};
	scs::Action Out{"Out", { Variable{"part"}, Object{"3"} }};
	scs::Action ApplyAdhesive{"ApplyAdhesive", { Variable{"part"}, Variable{"type"}, Object{"3"}}};

	scs::ActionProgram NopAp{Nop};
	scs::ActionProgram RadialDrillAp{RadialDrill};
	scs::ActionProgram AttachBit3mmAp{AttachBit3mm};
	scs::ActionProgram AttachBit5mmAp{AttachBit5mm};
	scs::ActionProgram ApplyAdhesiveAp{ApplyAdhesive};
	scs::ActionProgram InAp{In};
	scs::ActionProgram OutAp{Out};

	scs::Branch nd1{AttachBit3mmAp, AttachBit5mmAp};
	Formula cond = Predicate{"Equipped", {scs::Variable{"bit"}, Object{"3"} }};
	scs::CgIf if1{cond, RadialDrillAp, nd1};

	scs::Branch nd2(InAp, OutAp);
	Branch nd3(nd2, ApplyAdhesiveAp);
	Branch nd4(if1, nd3);

	// Objects and initial valuations
	s0.objects.emplace("3"); // Constant 3


	// Preconditions

	// Successors



	auto prog = std::make_shared<Loop>(nd4);
	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
