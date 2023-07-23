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
	In(part, 3) | Out(part, 3) | DetachBit(bit, 3)
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
	scs::Action DetachBit{"DetachBit", { Variable{"bit"}, Object{"3"} }};
	scs::Action ApplyAdhesive{"ApplyAdhesive", { Variable{"part"}, Variable{"type"}, Object{"3"}}};

	Branch nd1{ActionProgram{AttachBit3mm}, ActionProgram{AttachBit5mm}};
	Formula cond_pred = Predicate{"Bit", {scs::Variable{"bit"}, Object{"3"} }};
	Formula cond = Quantifier{ "e", cond_pred, QuantifierKind::Existential };
	CgIf if1{cond, ActionProgram{RadialDrill}, nd1};

	Branch nd2(ActionProgram{In}, ActionProgram{Out});
	Branch nd3(nd2, ActionProgram{ApplyAdhesive});
	Branch nd4(nd3, ActionProgram{DetachBit});

	Branch nd5(if1, nd4);

	// Objects and initial valuations
	s0.objects.emplace("3"); // Constant 3
	s0.objects.emplace("0.5");

	// Preconditions

	// Successors



	auto prog = std::make_shared<Loop>(nd5);
	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
