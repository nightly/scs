#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class FolQuantifierTest : public ::testing::Test {
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