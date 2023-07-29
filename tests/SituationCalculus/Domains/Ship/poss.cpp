#include <gtest/gtest.h>
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class ShipPossTest : public ::testing::Test {
protected:

	// scs::Action ship_act{ "ship", {scs::Variable{"x"}} };
	scs::RelationalFluent at{ "At" };
	scs::RelationalFluent is_loc{ "IsLoc" };

	scs::Situation s0;
	scs::BasicActionTheory bat;

	void SetUp() override {
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula pre_ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		bat.pre["ship"] = { std::vector<Term>{Variable{"x"}},  pre_ship_form};

		// Poss(arrive(x), s) ≡ ¬∃y.At(y, ShipDock, s) ∧ ¬∃l.At(x,l, s)
		Formula pre_arrive_form = BinaryConnective{ UnaryConnective{ Quantifier{scs::Variable{"y"}, Predicate{"At", {scs::Variable{"y"}, scs::Object{"ShipDock"}}},
			QuantifierKind::Existential}, UnaryKind::Negation }, UnaryConnective{ Quantifier{scs::Variable{"l"}, Predicate{"At", {scs::Variable{"x"}, scs::Variable{"l"}}},
			QuantifierKind::Existential}, UnaryKind::Negation }, BinaryKind::Conjunction };
		bat.pre["arrive"] = { std::vector<Term>{Variable{"x"}}, pre_arrive_form, };

		// Poss(move(x,l,l'), s) ≡ At(x,l, s) ∧ IsLoc(l') ∧ ¬∃y.At(y,l , s)
		Formula s1 = Predicate{ "At", {scs::Variable{"x"}, scs::Variable{"l"}}};
		Formula s2 = Predicate{ "IsLoc", {scs::Variable{"l'"}}};
		Formula s3 = UnaryConnective{ Quantifier{scs::Variable{"y"}, Predicate{"At", {scs::Variable{"y"}, scs::Variable{"l"}}}, QuantifierKind::Existential}, UnaryKind::Negation};
		Formula pre_move_form = BinaryConnective{BinaryConnective{s1, s2, BinaryKind::Conjunction}, s3, BinaryKind::Conjunction};

		bat.pre["move"] = { std::vector<Term>{Variable{"x"}, Variable{"l"}, Variable{"l'"}}, pre_move_form };

		s0.AddFluent(at);
		s0.AddFluent(is_loc);
		bat.SetInitial(s0);
	}

	// void TearDown() override {}
};


TEST_F(ShipPossTest, Ship) {
	Action ship_concrete_act{ "ship", { scs::Object{"crate1", bat}} };
	bool res = s0.Possible(ship_concrete_act, bat);
	EXPECT_EQ(res, false);

	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate1"}, scs::Object{"ShipDock"} }, true);
	s0.relational_fluents_["At"].AddValuation({ scs::Object{"crate2"}, scs::Object{"ShipDock"} }, true);
	res = s0.Possible(ship_concrete_act, bat);
	EXPECT_EQ(res, true);
	
	s0.PrintFluents();	
	// s0.PrintObjects();

	res = s0.Possible({ "ship", { scs::Object{"crate2", bat}} }, bat); 

	res = s0.Possible({ "ship", { scs::Object{"crate3", bat}} }, bat);
	EXPECT_EQ(res, false);
}

TEST_F(ShipPossTest, Arrive) {
	scs::SetConsoleEncoding();
	// std::cout << "Arrive test \n";
	// std::cout << bat.pre["arrive"].Form() << std::endl;
}

TEST_F(ShipPossTest, Move) {
	scs::SetConsoleEncoding();
	// std::cout << "Move test \n";

	// std::cout << bat.pre["move"].Form() << std::endl;

}
