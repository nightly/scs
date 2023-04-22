#include <gtest/gtest.h>
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"

using namespace scs;

class ShipMinimalSsaTest : public ::testing::Test {
protected:

	scs::Action move_act{ "move", {scs::Variable{"x"}, scs::Variable{"l"}, scs::Variable{"l'"}} };
	scs::Action ship_act{ "ship", {scs::Variable{"x"}} };

	scs::RelationalFluent at{ "At" };
	scs::RelationalFluent is_loc{ "IsLoc" };

	std::unordered_map<std::string, scs::Poss> pre;

	scs::Situation s0;
	scs::BasicActionTheory bat;


	void SetUp() override {
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		pre["ship"] = { ship_form, std::vector<Term>{Variable{"x"}} };

		is_loc.AddValuation({scs::Object{ "ShipYard" }}, true);
		is_loc.AddValuation({scs::Object{ "ShipDock" }}, true);
		s0.AddFluent(at);
		s0.AddFluent(is_loc);

		scs::Successor at_successor{ at, true, Formula{true} };

		bat.successors["at"] = at_successor;


		bat = { .initial = s0, .pre = pre };
	}

	// void TearDown() override {}
};

TEST_F(ShipMinimalSsaTest, Move) {
	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate1"}, scs::Object{"ShipDock"} }, true);
}