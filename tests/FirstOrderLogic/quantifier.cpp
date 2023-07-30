#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class FolQuantifierTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::BasicActionTheory bat;
	scs::Evaluator eval{ {s, bat} };

	void SetUp() override {
		scs::SetConsoleEncoding();
		bat.objects.emplace("robot1");
		bat.objects.emplace("robot2");
		scs::RelationalFluent Holding, Safe, Off, On;
		Holding.AddValuation({ "robot1" }, false);
		Holding.AddValuation({ "robot2" }, true);
		Safe.AddValuation({ "robot1" }, true);
		Safe.AddValuation({ "robot2" }, true);
		// 0-arity
		Off.AddValuation(false);
		On.AddValuation(true);

		s.AddFluent("Holding", Holding);
		s.AddFluent("Safe", Safe);
		s.AddFluent("Off", Off);
		s.AddFluent("On", On);
	}

};

TEST_F(FolQuantifierTest, Existential) {
	scs::Formula f = Quantifier("x", Predicate("Holding", std::vector<Term>{Variable{ "x" }}), QuantifierKind::Existential);
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST_F(FolQuantifierTest, Universal) {
	scs::Formula f = Quantifier("x", Predicate("Holding", std::vector<Term>{Variable{ "x" }}), QuantifierKind::Universal);
	bool result = std::visit(eval, f);
	EXPECT_EQ(result, false);

	f = Quantifier("x", Predicate("Safe", std::vector<Term>{Variable{ "x" }}), QuantifierKind::Universal);
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST_F(FolQuantifierTest, ExistentiallyUniversal) {

}
