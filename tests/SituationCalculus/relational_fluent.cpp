#include <gtest/gtest.h>
#include "scs/SituationCalculus/relational_fluent.h"


class RelationalFluentTest : public ::testing::Test {
protected:
	void SetUp() override {
		holding.AddValuation(true);
		holding_params.AddValuation(std::vector<scs::Object>{"robot", "plate"}, false);
	}

	scs::RelationalFluent holding{ "Holding" }, holding_params{ "HoldingP" };

	// void TearDown() override {}
};

TEST_F(RelationalFluentTest, Valuation) {
	EXPECT_EQ(holding.Valuation(), true);
	holding_params.AddValuation(std::vector<scs::Object>{"robot", "plate2"}, true);
	EXPECT_EQ(holding_params.Valuation(std::vector<scs::Object>{"robot", "plate2"}), true);
}

TEST_F(RelationalFluentTest, MissingValuation) {
	EXPECT_EQ(holding_params.Valuation(std::vector<scs::Object>{"robot10", "plate"}), false);
}

TEST_F(RelationalFluentTest, UpdateValuation) {
	EXPECT_EQ(holding_params.Valuation({ {"robot", "plate"} }), false);
	holding_params.AddValuation(std::vector<scs::Object>{"robot", "plate"}, true);
	EXPECT_EQ(holding_params.Valuation({ {"robot", "plate"} }), true);
}

TEST_F(RelationalFluentTest, ToString) {
	ASSERT_EQ(holding.ToString(), "Holding = true");
	ASSERT_EQ(holding_params.ToString(), "HoldingP(robot,plate) = false");
}

TEST_F(RelationalFluentTest, ContainsObject) {
	scs::Object o1("disc");
	scs::Object o2("plate");
	EXPECT_EQ(holding_params.ContainsObject(o1), false);
	EXPECT_EQ(holding_params.ContainsObject(o2), true);
}

TEST_F(RelationalFluentTest, TypeAssertion) {
	EXPECT_DEATH(holding.AddValuation({ "t1" }, false), "Sc: Relational Fluent is inconsistent with parameters having none and some");
}