#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class FolEqualityTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::BasicActionTheory bat;
	scs::Evaluator eval{ {s, bat} };

	void SetUp() override {
		scs::SetConsoleEncoding();
		bat.objects.emplace("robot1");
		bat.objects.emplace("robot2");
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
	scs::BasicActionTheory bat;
	bat.objects.emplace("obj1");
	scs::Evaluator eval2{ {s2, bat} };

	Formula f = Quantifier("x", BinaryConnective(scs::Object{ "obj1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Universal);
	bool result = std::visit(eval2, f);
	EXPECT_EQ(result, true);
}

TEST(FolEqEval, EqualityNewObject) {
	// TODO: possibly consider adding domain object to the situation when constructing it by pointer.
	scs::Situation s;
	scs::BasicActionTheory bat;
	Formula f = Quantifier("x", BinaryConnective(scs::Object{ "obj1", bat }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Universal);
	scs::Evaluator eval{{s, bat}};
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
	Action expected{"loadHeavy", std::vector<Term>{scs::Object{"part1"}, scs::Object{"glue"}}};

	Formula a1 = Action{ "loadHeavy" };
	Formula a2 = Action{ "load" };

	Formula a3 = Action{ "loadHeavy", std::vector<Term>{scs::Object{"part1"}, scs::Object{"glue"}}};
	Formula a4 = Action{ "loadHeavy", std::vector<Term>{scs::Variable{"v_part2"}, scs::Object{"glue"}}};
	Formula a5 = Action{ "loadHeavy", std::vector<Term>{scs::Object{"part1"}, scs::Object{"glue1"}} };
	Formula a6 = Action{ "loadHeavy", std::vector<Term>{scs::Variable{"v_part1"}, scs::Object{"glue"}} };

	scs::FirstOrderAssignment assignment;
	assignment.Set(scs::Variable{"a"}, expected);

	assignment.Set(scs::Variable{"v_part1"}, scs::Object{"part1"});
	assignment.Set(scs::Variable{"v_part2"}, scs::Object{"part2"});
	scs::Evaluator eval{assignment};

	Formula check1 = BinaryConnective(a1, scs::Variable("a"), BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check1), false);
	Formula check2 = BinaryConnective(scs::Variable("a"), a2, BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check2), false);

	Formula check3 = BinaryConnective(a3, scs::Variable("a"), BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check3), true);
	Formula check4 = BinaryConnective(scs::Variable("a"), a3, BinaryKind::Equal);
	EXPECT_EQ(std::visit(eval, check4), true);
	
	Formula check5 = BinaryConnective(a4, scs::Variable("a"), BinaryKind::NotEqual);
	EXPECT_EQ(std::visit(eval, check5), true);
	Formula check6 = BinaryConnective(a5, scs::Variable("a"), BinaryKind::NotEqual);
	EXPECT_EQ(std::visit(eval, check6), true);

	Formula check7 = BinaryConnective(a6, scs::Variable("a"), BinaryKind::NotEqual);
	EXPECT_EQ(std::visit(eval, check7), false);
}
