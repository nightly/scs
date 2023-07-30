#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

inline void ship_prog() {
	scs::Action move_act{ "move", {scs::Variable{"x"}, scs::Variable{"l"}, scs::Variable{"l'"}} };
	scs::Action arrive_act{ "arrive", {scs::Variable{"x"}} };
	scs::Action ship_act{ "ship", {scs::Variable{"x"}} };

	scs::RelationalFluent at{};
	scs::RelationalFluent is_loc{};

	ankerl::unordered_dense::map<std::string, scs::Poss> pre;
	scs::Situation s0;
	scs::BasicActionTheory bat;

	// Poss(move(x,l,l ), s) ≡ At(x,l, s) ∧ IsLoc(l ) ∧ ¬∃y.At(y,l , s)
	pre["move"] = { true };

	// Poss(arrive(x), s) ≡ ¬∃y.At(y, ShipDock, s) ∧ ¬∃l.At(x,l, s)
	pre["arrive"] = { true };

	// Poss(ship(x), s) ≡ At(x, ShipDock, s)
	Formula ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
	pre["ship"] = { std::vector<Term>{Variable{"x"}}, ship_form};
	
	bat.objects.emplace("ShipDock");

	s0.AddFluent("at", at);
	s0.AddFluent("IsLoc", is_loc);

	bat.pre = pre;
	bat.SetInitial(s0);


	/*
	*****************
	*/

	Action ship_concrete_act{ "ship", { scs::Object{"crate1", bat}} };
	s0.Possible(ship_concrete_act, bat);

}