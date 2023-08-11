#pragma once

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

	inline Resource HingeResource1() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Nop{ "Nop", {} };
		scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
		scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
		scs::Action Release{ "Release", { Variable{"part"}, Object{"1"} }};
		scs::Action Store{ "Store", { Variable{"part"}, Variable{"code"}, scs::Object{"1"} } };

		ret.bat.types["Clamp"] = ActionType::Prepatory;
		ret.bat.types["Release"] = ActionType::Prepatory;
		ret.bat.types["Store"] = ActionType::Manufacturing;

		ret.bat.objects.emplace("ok");

		Branch nd1(ActionProgram{ Nop }, Sequence(ActionProgram{ Clamp }, ActionProgram{ Release }));
		Loop l1(nd1);
		Sequence s1(ActionProgram{In}, l1);
		Sequence s2(s1, Branch(ActionProgram{Out}, ActionProgram{Store}));

		Branch nd2(ActionProgram{ Nop }, s2);

		// Objects and initial valuations
		ret.bat.objects.emplace("1"); // Constant 1
		ret.bat.objects.emplace("ok");

		// Preconditions
		Formula pre_clamp = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
			Not(Quantifier("f", Predicate("clamped", {Variable{"part"}, Variable{"f"}, Variable{"i"}}), QuantifierKind::Existential));
		ret.bat.pre["Clamp"] = { std::vector<Term>{Variable{"part"}, Variable{"force"}, Variable{"i"}}, pre_clamp};

		Formula pre_release = Predicate("part", { Variable{"part"} }) && Predicate("at", {Variable{"part"}, Variable{"i"}}) &&
			Quantifier("f", Predicate("clamped", {Variable{"part"}, Variable{"f"}, Variable{"i"}}), QuantifierKind::Existential);
		ret.bat.pre["Release"] = { std::vector<Term>{Variable{"part"}, Variable{"i"}}, pre_release };

		Formula pre_store = Predicate("part", { scs::Variable{"part"} }) && Predicate("at", { Variable{"part"}, Variable{"i"} }) &&
			Predicate("status", { Variable{"code"} });
		ret.bat.pre["Store"] = { std::vector<Term>{Variable{"part"}, Variable{"code"}, Variable{"i"}}, pre_store };

		// Successors
		Formula clamp_pos = a_eq(scs::Action{"Clamp", { Variable{"part"}, Variable{"force"}, Variable{"i"} }});
		Formula clamp_neg = cv() && Not(a_eq(scs::Action{"Release", {Variable{"part"}, Variable{"i"}}}));
		ret.bat.successors["clamped"] = { {Variable{"part"}, Variable{"force"}, Variable{"i"}}, clamp_pos || clamp_neg};

		Formula part_neg = cv() && Not(Quantifier("code", a_eq(scs::Action{ "Remove", { Variable{"part"}, Variable{"code"},
			Variable{"i"} } }), QuantifierKind::Existential)) && Not(Quantifier("p", Quantifier("i", a_eq(Action{ "ApplyAdhesive",
				{Variable{"part"}, Variable{"p"}, Variable{"i"} } }), QuantifierKind::Existential), QuantifierKind::Existential));
		ret.bat.successors["part"] = { {Variable{"part"}}, part_neg };
		
		// on_site ssa in resource2.h


		ret.program = std::make_shared<Loop>(nd2);
		ret.bat.SetInitial(s0);
		return ret;
	}

}