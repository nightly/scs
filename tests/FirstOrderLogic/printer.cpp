#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/windows.h"

using namespace scs;


class FolPrinterTest : public ::testing::Test {
protected:
	void SetUp() override {
		SetConsoleEncoding();
	}

	// void TearDown() override {}
};

TEST_F(FolPrinterTest, Formula1) {
	std::ostringstream output;

	Printer v(output);
	Formula f = BinaryConnective(false, BinaryConnective(true, false, BinaryKind::Conjunction), BinaryKind::Disjunction);
	std::visit(v, f);
	// std::cout << f;

	EXPECT_EQ(output.str(), "(False ∨ (True ∧ False))");
}