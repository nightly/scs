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

	scs::RelationalFluent at{ "At" };
	scs::RelationalFluent is_loc{ "IsLoc" };

	std::unordered_map<std::string, scs::Poss> pre;
	scs::Situation s0;
	scs::BasicActionTheory bat;

	// Poss(move(x,l,l ), s) ≡ At(x,l, s) ∧ IsLoc(l ) ∧ ¬∃y.At(y,l , s)
	pre["move"] = { true };

	// Poss(arrive(x), s) ≡ ¬∃y.At(y, ShipDock, s) ∧ ¬∃l.At(x,l, s)
	pre["arrive"] = { true };

	// Poss(ship(x), s) ≡ At(x, ShipDock, s)
	Formula ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
	pre["ship"] = { ship_form, std::vector<Term>{Variable{"x"}} };
	
	s0.objects.emplace("ShipDock");

	s0.AddFluent(at);
	s0.AddFluent(is_loc);

	bat = { .initial = s0, .pre = pre };


	/*
	*****************
	*/

	Action ship_concrete_act{ "ship", { scs::Object{"crate1", s0}} };
	s0.Possible(ship_concrete_act, pre["ship"]);

}