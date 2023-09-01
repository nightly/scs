﻿#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#BAT

#Program
loop:
	Nop* | In(part, 1); (Nop | (Clamp(part, force, 1); Release(part, 1))*; (Out(part, 1) | Store(part, code, 1))
*/

namespace scs::examples {

	inline Resource HingeGroundedResource1() {
		Resource ret;
		Situation s0;

		// Program ----
		scs::Action Nop{ "Nop", {} };
		scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
		scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
		scs::Action Release{ "Release", { Variable{"part"}, Object{"1"} }};
		scs::Action Store{ "Store", { Variable{"part"}, Variable{"code"}, Object{"1"} } };
		// Grounded
		ActionProgram InG1{ Action{"In", {Object{"brass"}, Object{"1"}}} };
		ActionProgram InG2{ Action{"In", {Object{"tube"}, Object{"1"}}} };
		Branch InB(InG1, InG2);
		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"1"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"1"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram StoreG1(Action{ "Store", { Object{"brass"}, Object{"ok"}, Object{"1"}}});
		ActionProgram StoreG2(Action{ "Store", { Object{"tube"}, Object{"ok"}, Object{"1"}}});
		Branch StoreB(StoreG1, StoreG2);
		ActionProgram ClampG1(Action{ "Clamp", { Object{"brass"}, Object{"5"}, Object{"1"}}});
		ActionProgram ClampG2(Action{ "Clamp", { Object{"tube"}, Object{"5"}, Object{"1"}}});
		Branch ClampB(ClampG1, ClampG2);
		ActionProgram ReleaseG1(Action{"Release", {Object{"tube"}, Object{"1"}}});
		ActionProgram ReleaseG2(Action{"Release", {Object{"brass"}, Object{"1"}}});
		Branch ReleaseB(ReleaseG1, ReleaseG2);
		// ----
		ret.bat.types["Clamp"] = ActionType::Prepatory;
		ret.bat.types["Release"] = ActionType::Prepatory;
		ret.bat.types["Store"] = ActionType::Manufacturing;

		ret.bat.objects.emplace("ok");

		

		Branch nd1(ActionProgram{ Nop }, Sequence(ClampB, ReleaseB));
		Loop l1(nd1);
		Sequence s1(InB, l1);
		Sequence s2(s1, Branch(OutB, StoreB));

		Branch nd2(ActionProgram{ Nop }, s2);

		// Objects and initial valuations
		ret.bat.objects.emplace("1"); // Constant 1
		ret.bat.objects.emplace("ok");

		// Initial fluents
		s0.relational_fluents_["safe_force"].AddValuation({ Object{"brass"}, Object{"5"} }, true);
		s0.relational_fluents_["safe_force"].AddValuation({ Object{"tube"}, Object{"5"} }, true);
		s0.relational_fluents_["safe_force"].AddValuation({ Object{"tube"}, Object{"0.5"} }, true);

		// Preconditions
		Formula pre_clamp = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
			Predicate("safe_force", {Variable{"part"}, Variable{"force"}}) &&
			Not(Quantifier("f", Predicate("clamped", {Variable{"part"}, Variable{"f"}, Variable{"i"}}), QuantifierKind::Existential));
		ret.bat.pre["Clamp"] = { std::vector<Term>{Variable{"part"}, Variable{"force"}, Variable{"i"}}, pre_clamp};

		Formula pre_release = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
			Quantifier("f", Predicate("clamped", {Variable{"part"}, Variable{"f"}, Variable{"i"}}), QuantifierKind::Existential);
		ret.bat.pre["Release"] = { std::vector<Term>{Variable{"part"}, Variable{"i"}}, pre_release };

		Formula pre_store = Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
			Predicate("status", { Variable{"code"} });
		ret.bat.pre["Store"] = { std::vector<Term>{Variable{"part"}, Variable{"code"}, Variable{"i"}}, pre_store };

		// Successors
		Formula clamp_pos = a_eq(scs::Action{"Clamp", { Variable{"part"}, Variable{"force"}, Variable{"i"} }});
		Formula clamp_neg = cv() && Not(a_eq(scs::Action{"Release", {Variable{"part"}, Variable{"i"}}}));
		ret.bat.successors["clamped"] = { {Variable{"part"}, Variable{"force"}, Variable{"i"}}, clamp_pos || clamp_neg};

		Formula part_neg = cv() && Not(Quantifier("i", Quantifier("code", a_eq(scs::Action{ "Store", {Variable{"part"}, Variable{"code"},
			Variable{"i"} } }), QuantifierKind::Existential), QuantifierKind::Existential)) &&
			Not(Quantifier("other", Quantifier("i", a_eq(Action{ "ApplyAdhesive",
				{Variable{"other"}, Variable{"part"}, Variable{"i"} } }), QuantifierKind::Existential), QuantifierKind::Existential));
		ret.bat.successors["part"] = { {Variable{"part"}}, part_neg };
		
		// on_site ssa in resource2.h


		ret.program = std::make_shared<Loop>(nd2);
		ret.bat.SetInitial(s0);
		return ret;
	}

	inline CharacteristicGraph HingeGroundedResource1Cg() {
		// Simplified Cg (manually for now)
		CharacteristicGraph cg;
		cg.lts.AddTransition(0,{Action{"Nop"}, true }, 0);


		cg.lts.AddTransition(2,{ Action{ "Clamp", { Object{"brass"}, Object{"5"}, Object{"1"}}}, true }, 4);
		cg.lts.AddTransition(2,{ Action{ "Clamp", { Object{"tube"}, Object{"5"}, Object{"1"}}}, true }, 4);

		cg.lts.AddTransition(4, { Action{"Release", {Object{"brass"}, Object{"1"}}}, true}, 2);
		cg.lts.AddTransition(4, { Action{"Release", {Object{"tube"}, Object{"1"}}}, true}, 2);

		cg.lts.AddTransition(0, { Action{"In", {Object{"brass"}, Object{"1"}}}, true}, 2);
		cg.lts.AddTransition(0, { Action{"In", {Object{"tube"}, Object{"1"}}}, true}, 2);
		
		cg.lts.AddTransition(2, { Action{"Out", {Object{"brass"}, Object{"1"}}}, true }, 0);
		cg.lts.AddTransition(2, { Action{"Out", {Object{"tube"}, Object{"1"}}}, true }, 0);
		
		cg.lts.AddTransition(2, { Action{ "Store", { Object{"brass"}, Object{"ok"}, Object{"1"}}}, true}, 0);
		cg.lts.AddTransition(2, { Action{ "Store", { Object{"tube"}, Object{"ok"}, Object{"1"}}}, true}, 0);
		return cg;
	}

}