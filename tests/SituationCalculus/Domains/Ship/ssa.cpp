#include <gtest/gtest.h>
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"

using namespace scs;

class ShipSsaTest : public ::testing::Test {
protected:

	scs::Action move_act{ "move", {scs::Variable{"x"}, scs::Variable{"l"}, scs::Variable{"l'"}} };
	scs::Action ship_act{ "ship", {scs::Variable{"x"}} };

	scs::Action new_loc_act{ "CreateLoc", {scs::Variable{"x"}}};

	scs::RelationalFluent at{ "At" };
	scs::RelationalFluent is_loc{ "IsLoc" };

	scs::Situation s0;
	scs::BasicActionTheory bat;


	void SetUp() override {
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		bat.pre["ship"] = { ship_form, std::vector<Term>{Variable{"x"}} };

		is_loc.AddValuation({scs::Object{ "ShipYard" }}, true);
		is_loc.AddValuation({scs::Object{ "ShipDock" }}, true);
		s0.AddFluent(at);
		s0.AddFluent(is_loc);


		// Poss(move(x,l,l'), s) ≡ At(x,l, s) ∧ IsLoc(l') ∧ ¬∃y.At(y,l , s)
		Formula s1 = Predicate{ "At", {scs::Variable{"x"}, scs::Variable{"l"}} };
		Formula s2 = Predicate{ "IsLoc", {scs::Variable{"l'"}} };
		Formula s3 = UnaryConnective{ Quantifier{scs::Variable{"y"}, Predicate{"At", {scs::Variable{"y"}, scs::Variable{"l"}}}, QuantifierKind::Existential}, UnaryKind::Negation };
		Formula pre_move_form = BinaryConnective{ BinaryConnective{s1, s2, BinaryKind::Conjunction}, s3, BinaryKind::Conjunction };

		bat.pre["move"] = { pre_move_form, std::vector<Term>{Variable{"x"}, Variable{"l"}, Variable{"l'"}} };

		// @Successor At
		// Positive effect: new location
		// Negative effect: no longer at old location
		// Neutral: current valuation
		// Formula move_ssa_neutral = Predicate{ "At", {scs::Variable{"x"}, scs::Variable{"loc"}}}; // Requires substitution at the progression-update level


		scs::Successor at_successor{ at, true, Formula{true} };
		bat.successors["at"] = at_successor;

		// @Successor IsLoc
		// Positive effect: create new location
		// Negative effect: none. If the object cannot be a location, i.e. it is of some other type, that is determined by preconditions.
		// Neutral: current valuation.
		Formula loc_pos_1 = BinaryConnective{ scs::Variable{"a"}, scs::Action{"CreateLoc"} , BinaryKind::Equal};
		scs::Successor is_loc_successor{ is_loc, true, loc_pos_1 };
		bat.successors["isLoc"] = is_loc_successor;

		bat.initial = s0;
	}

	// void TearDown() override {}
};

TEST_F(ShipSsaTest, NewLoc) {


}

TEST_F(ShipSsaTest, Move) {
	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate1"}, scs::Object{"ShipDock"} }, true);
}