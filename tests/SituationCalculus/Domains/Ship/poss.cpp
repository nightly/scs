#include <gtest/gtest.h>
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"

using namespace scs;

class ShipMinimalPossTest : public ::testing::Test {
protected:

	// scs::Action ship_act{ "ship", {scs::Variable{"x"}} };
	scs::RelationalFluent at{ "At" };
	scs::RelationalFluent is_loc{ "IsLoc" };

	std::unordered_map<std::string, scs::Poss> pre;
	scs::Situation s0;
	scs::BasicActionTheory bat;

	void SetUp() override {
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		pre["ship"] = { ship_form, std::vector<Term>{Variable{"x"}} };

		s0.AddFluent(at);
		s0.AddFluent(is_loc);
		bat = { .initial = s0, .pre = pre };
	}

	// void TearDown() override {}
};

TEST_F(ShipMinimalPossTest, Initial) {
	Action ship_concrete_act{ "ship", { scs::Object{"crate1", s0}} };
	bool res = s0.Possible(ship_concrete_act, pre["ship"]);
	EXPECT_EQ(res, false);

	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate1"}, scs::Object{"ShipDock"} }, true);
	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate2"}, scs::Object{"ShipDock"} }, true);
	res = s0.Possible(ship_concrete_act, pre["ship"]);
	EXPECT_EQ(res, true);
	
	// s0.PrintFluents();	
	// s0.PrintObjects();

	res = s0.Possible({ "ship", { scs::Object{"crate2", s0}} }, pre["ship"]); // Note that this works even though we forgot to add crate2 explicitly to the domain...
	// So we can double validate whether objects exist before doing valuation lookups. But it's program correctness vs user correctness.

	res = s0.Possible({ "ship", { scs::Object{"crate3", s0}} }, pre["ship"]);
	EXPECT_EQ(res, false);
}