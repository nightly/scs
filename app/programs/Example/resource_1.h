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
	Nop | (In(part, 1); (Nop | Clamp(part, force, 1); Release(part, 1))∗ ; Out(part, 1)
*/

inline Resource ExampleResource1() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop", {} };
	scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
	scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
	scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
	scs::Action Release{ "Release", { Variable{"part"}, Object{"1"} }};

	Branch nd1(ActionProgram{Nop}, Sequence(ActionProgram{Clamp}, ActionProgram{Release}));
	Loop l1(nd1);
	Sequence s1(ActionProgram{In}, l1);
	Sequence s2(s1, ActionProgram{Out});

	Branch nd2(ActionProgram{Nop}, s2);

	// Objects and initial valuations
	s0.objects.emplace("1"); // Constant 1

	// Preconditions
	Formula pre_clamp = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
		!Predicate("clamped", {Variable{"part"}, Variable{"i"}});
	ret.bat.pre["Clamp"] = { std::vector<Term>{Variable{"part"}, Variable{"force"}, Variable{"i"}}, pre_clamp};

	Formula pre_release = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
		Predicate("clamped", {Variable{"part"}});
	ret.bat.pre["Release"] = { std::vector<Term>{Variable{"part"}, Variable{"i"}}, pre_release };

	// Successors
	Formula clamp_pos = a_eq(scs::Action{"Clamp", { Variable{"part"}, Variable{"force"}, Variable{"i"} }});
	Formula clamp_neg = cv() && !a_eq(scs::Action{"Release", {Variable{"part"}, Variable{"i"}}});
	ret.bat.successors["clamped"] = { {Variable{"part"}, Variable{"i"}}, clamp_pos || clamp_neg };

	ret.program = std::make_shared<Loop>(nd2);
	ret.bat.SetInitial(s0);
	return ret;
}
