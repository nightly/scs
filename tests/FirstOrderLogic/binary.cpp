#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/windows.h"

using namespace scs;

class FolEvaluatorTest : public ::testing::Test {
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

TEST_F(FolEvaluatorTest, BooleanIdentity) {
	scs::Formula f = true;
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = false;
	result = std::visit(eval, f);
	EXPECT_EQ(result, false);
}

TEST_F(FolEvaluatorTest, CombinedFormula) {
	// (False) or (True and False)
	Formula f = BinaryConnective(false, BinaryConnective(true, false, BinaryKind::Conjunction), BinaryKind::Disjunction);
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, false);
}

