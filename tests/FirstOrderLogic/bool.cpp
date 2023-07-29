#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class FolEvaluatorTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::BasicActionTheory bat;
	scs::Evaluator eval{ {s, bat} };

	void SetUp() override {
		scs::SetConsoleEncoding();
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

