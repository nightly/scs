#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/windows.h"

using namespace scs;

class FolUnaryTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::BasicActionTheory bat;
	scs::Evaluator eval{ {s, bat} };

	void SetUp() override {
		scs::SetConsoleEncoding();
	}

};

TEST_F(FolUnaryTest, Negation) {
	scs::Formula f = UnaryConnective(true, UnaryKind::Negation);
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, false);

	f = UnaryConnective(false, UnaryKind::Negation);
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}