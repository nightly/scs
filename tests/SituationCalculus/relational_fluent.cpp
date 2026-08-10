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
	ASSERT_EQ(holding_params.ToString(), "");
}

TEST_F(RelationalFluentTest, StoresOnlyTrueTuples) {
	holding_params.AddValuation({"robot", "plate"}, true);
	ASSERT_EQ(holding_params.TrueTuples().size(), 1);

	holding_params.AddValuation({"robot", "plate"}, false);
	EXPECT_TRUE(holding_params.TrueTuples().empty());
	EXPECT_FALSE(holding_params.Valuation({"robot", "plate"}));
}

TEST_F(RelationalFluentTest, ZeroArityFalseIsAnEmptyExtension) {
	holding.AddValuation(false);
	EXPECT_FALSE(holding.Valuation());
	EXPECT_TRUE(holding.TrueTuples().empty());
	EXPECT_EQ(holding.ToString(), "Fluent = false");
}

TEST_F(RelationalFluentTest, TypeAssertion) {
	EXPECT_THROW(holding.AddValuation({ "t1" }, false), std::invalid_argument);
}

TEST_F(RelationalFluentTest, EqualityAndHashIgnoreInsertionOrder) {
	scs::RelationalFluent first;
	first.AddValuation({"robot", "plate"}, true);
	first.AddValuation({"robot", "cup"}, false);

	scs::RelationalFluent second;
	second.AddValuation({"robot", "cup"}, false);
	second.AddValuation({"robot", "plate"}, true);

	EXPECT_EQ(first, second);
	EXPECT_EQ(std::hash<scs::RelationalFluent>{}(first), std::hash<scs::RelationalFluent>{}(second));
	EXPECT_FALSE(first != second);
}

TEST_F(RelationalFluentTest, ArityParticipatesInEquality) {
	EXPECT_NE(scs::RelationalFluent(1), scs::RelationalFluent(2));
}
