#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

#include "common.h"

using namespace scs;

/*
#BAT

#Program
loop:
	if equipped(bit, 3) then
		RadialDrill(part, bit, diameter, 3)
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
	scs::Action RadialDrill{"RadialDrill", { Variable{"part"},  Variable{"bit"}, Variable{"diameter"}, Object{"3"} }};
	scs::Action AttachBit3mm{"AttachBit", { Object{"3mm"}, Object{"3"} }}; // concrete action
	scs::Action AttachBit5mm{"AttachBit", { Object{"5mm"}, Object{"3"} }}; // concrete action
	scs::Action In{"In", { Variable{"part"}, Object{"3"} }};
	scs::Action Out{"Out", { Variable{"part"}, Object{"3"} }};
	scs::Action DetachBit{"DetachBit", { Variable{"bit"}, Object{"3"} }};
	scs::Action ApplyAdhesive{"ApplyAdhesive", { Variable{"part"}, Variable{"type"}, Object{"3"}}};

	Branch nd1{ActionProgram{AttachBit3mm}, ActionProgram{AttachBit5mm}};
	Formula cond_pred = Predicate{"equipped_bit", {scs::Variable{"b"}, Object{"3"} }};
	Formula cond = Quantifier{ "b", cond_pred, QuantifierKind::Existential };
	CgIf if1{cond, ActionProgram{RadialDrill}, nd1};

	Branch nd2(ActionProgram{In}, ActionProgram{Out});
	Branch nd3(nd2, ActionProgram{ApplyAdhesive});
	Branch nd4(nd3, ActionProgram{DetachBit});

	Branch nd5(if1, nd4);

	// Objects and initial valuations
	s0.objects.emplace("3"); // Constant 3
	s0.objects.emplace("0.5");

	// Preconditions
	Formula pre_detach = Predicate("equipped_bit", {scs::Variable{"bit"}, scs::Variable{"i"}});
	ret.bat.pre["DetachBit"] = { {scs::Variable{"bit"}, scs::Variable{"i"}}, pre_detach};

	ExampleCommon com;
	Formula pre_drill = Predicate("material", { Variable{"part"} }) && Predicate("suitable", { Variable{"bit"}, Variable{"diameter"} }) &&
		Quantifier("c", Predicate("clamped", {Variable{"part"}, Variable{"c"}}), QuantifierKind::Existential) && com.within_reach;
	// add within_reach and remove clamped's i 
	ret.bat.pre["RadialDrill"] = { {Variable{"part"}, Variable{"part"}, Variable{"diameter"}, Variable{"i"}}, pre_drill};

	Formula pre_apply_adhesive = true;
	ret.bat.pre["ApplyAdhesive"] = { {}, pre_apply_adhesive };

	ret.bat.pre["AttachBit"] = { {}, true };

	// Successors
	Formula bit_pos = a_eq(Action("AttachBit", {Variable{"bit"}, Variable{"i"}}));
	Formula bit_neg = cv() || a_neq(Action("DetachBit", {Variable{"bit"}, Variable{"i"}} ));
	ret.bat.successors["equipped_bit"] = { {Variable{"bit"}, Variable{"i"}}, bit_pos || bit_neg};

	auto prog = std::make_shared<Loop>(nd5);
	ret.program = prog;
	ret.bat.SetInitial(s0);
	return ret;
}
