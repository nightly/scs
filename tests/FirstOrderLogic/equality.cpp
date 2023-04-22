#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class FolEqualityTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::Evaluator eval{ s };

	void SetUp() override {
		scs::SetConsoleEncoding();
		s.objects.emplace("robot1");
		s.objects.emplace("robot2");
		scs::RelationalFluent Holding{ "Holding" }, Safe{ "Safe" }, Off{ "Off" }, On{ "On" };
		Holding.AddValuation({ "robot1" }, false);
		Holding.AddValuation({ "robot2" }, true);
		Safe.AddValuation({ "robot1" }, true);
		Safe.AddValuation({ "robot2" }, true);
		// 0-arity
		Off.AddValuation(false);
		On.AddValuation(true);

		s.AddFluent(Holding);
		s.AddFluent(Safe);
		s.AddFluent(Off);
		s.AddFluent(On);
	}

};

TEST_F(FolEqualityTest, Equality) {
	scs::Formula f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot1" }, BinaryKind::Equal);
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot2" }, BinaryKind::Equal);
	result = std::visit(eval, f);
	EXPECT_EQ(result, false);

	f = Quantifier("x", BinaryConnective(scs::Object{ "robot1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Existential);
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST_F(FolEqualityTest, EqualityNewObjectFixture) {

}

TEST_F(FolEqualityTest, NotEqual) {
	scs::Formula f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot2" }, BinaryKind::NotEqual);
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = Quantifier("x", BinaryConnective(scs::Object{ "robot1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Universal);
	result = std::visit(eval, f);
	EXPECT_EQ(result, false);
}

TEST(FolEqEval, Equality) {
	scs::Situation s2;
	s2.objects.emplace("obj1");
	scs::Evaluator eval2{ s2 };

	Formula f = Quantifier("x", BinaryConnective(scs::Object{ "obj1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Universal);
	bool result = std::visit(eval2, f);
	EXPECT_EQ(result, true);
}

TEST(FolEqEval, EqualityNewObject) {
	// TODO: possibly consider adding domain object to the situation when constructing it by pointer.
	scs::Situation s;
	Formula f = Quantifier("x", BinaryConnective(scs::Object{ "obj1", s }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Universal);
	scs::Evaluator eval{s};
	bool result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST(FolEqEval, EqualityActions) {
	Formula a1 = Action{ "load" };
	Formula a2 = Action{ "load" };
	Formula a3 = Action{ "loadHeavy" };
	// UNA assumption.
	// Currently these actions don't even have to form part of the domain to be checked upon since we don't compare action terms (not needed as of now), only types.
	scs::Evaluator eval{};
	Formula check = BinaryConnective(a1, a2, BinaryKind::Equal);
	bool result = std::visit(eval, check);
	EXPECT_EQ(result, true);

	Formula check2 = UnaryConnective(BinaryConnective(a1, a3, BinaryKind::Equal), UnaryKind::Negation);
	bool result2 = std::visit(eval, check2);
	EXPECT_EQ(result2, true);
}

TEST(FolEqEval, EqualityActionsVariable) {
	Formula a1 = Action{ "load" };
	Formula a2 = Action{ "load" };
	Formula a3 = Action{ "loadHeavy" };
	scs::FirstOrderAssignment assign;
	assign.Set(scs::Variable{"a"}, Action("loadHeavy"));
	scs::Evaluator eval{assign};
	
	// @Future: this doesn't do termwise equality checks just UNA. This can be done by updating the Action type to handle this with == or custom function in evaluator
	// that maps variables to terms. In general it's not needed to compare action terms, just the current action type which is done with UNA.

	Formula check1 = BinaryConnective(a3, scs::Variable("a"), BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check1), true);

	Formula check2 = BinaryConnective(scs::Variable("a"), a3, BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check2), true);

	Formula check3 = BinaryConnective(a1, scs::Variable("a"), BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check3), false);

	Formula check4 = BinaryConnective(a1, scs::Variable("a"), BinaryKind::NotEqual);
	EXPECT_EQ(std::visit(eval, check4), true);
}

TEST(FolEqEval, EqualityActionsWithTerms) {
	// Below requires ActionSort
	// Action{"load", std::vector<Term>{scs::Variable{"part"}, scs::Variable{"loc"}}};
}