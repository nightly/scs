#include <gtest/gtest.h>
#include "scs/SituationCalculus/situation.h"

class SituationTest : public ::testing::Test {
protected:

	scs::Action a1{ "safetySwitch" };
	scs::Action a2{ "offSwitch" };

	scs::Situation s0{};
	scs::Situation s1{};
	scs::Situation s2{};

	void SetUp() override {
		// Ideally, this should be done differently instead of emplacing directly into the history vector.
		s1.history.emplace_back(a1);
		s2 = s1;
		s2.history.emplace_back(a2);
	}

};


TEST_F(SituationTest, Equality) {
	scs::Situation sit0{};
	EXPECT_EQ(sit0, s0);

	scs::Situation sit1{};
	sit1.history.emplace_back(a1);
	EXPECT_EQ(sit1, s1);
}

TEST_F(SituationTest, Inequality) {
	EXPECT_NE(s0, s1);
	EXPECT_NE(s0, s2);
	EXPECT_NE(s1, s2);
}

TEST_F(SituationTest, PrintEmpty) {
	std::ostringstream output;
	scs::Situation s;
	output << s;
	std::string expected = R"(do[]:
 Fluents = 
)";
	EXPECT_EQ(output.str(), expected);
}

TEST_F(SituationTest, PrintWithHistory) {
	std::ostringstream output;
	output << s2;
	std::string expected = R"(do[safetySwitch(), offSwitch()]:
 Fluents = 
)";
	EXPECT_EQ(output.str(), expected);
}

TEST_F(SituationTest, Length) {
	EXPECT_EQ(s0.Length(), 0);
	EXPECT_EQ(s2.Length(), 2);
}