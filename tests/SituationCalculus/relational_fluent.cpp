#include <gtest/gtest.h>
#include "scs/SituationCalculus/relational_fluent.h"


class RelationalFluentTest : public ::testing::Test {
protected:
	void SetUp() override {
		holding.AddValuation(true);
		holding_params.AddValuation(std::vector<scs::Object>{"robot", "plate"}, false);
	}

	scs::RelationalFluent holding, holding_params;

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
	ASSERT_EQ(holding.ToString(), "Fluent = true");
	ASSERT_EQ(holding_params.ToString(), "(robot,plate) = false");
}

TEST_F(RelationalFluentTest, TypeAssertion) {
	EXPECT_DEATH(holding.AddValuation({ "t1" }, false), "Adding valuation to fluent that has previously set different arity");
}