#include <gtest/gtest.h>

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

class DatabaseSsaTest : public ::testing::Test {
protected:
	scs::BasicActionTheory bat;
	scs::Situation s0;
	scs::RelationalFluent enrolled{"enrolled"};
	scs::Action register_act{"register", {Variable{"st"}}}; // Abstract action type for SSA
	scs::Action unregister_act{"unregister", {Variable{"st"}}}; // Abstract action type for SSA

	scs::Object doe{"Doe", s0};
	scs::Object john{"John", s0};

	void SetUp() override {
		scs::SetConsoleEncoding();
		enrolled.AddValuation({ john }, false);
		enrolled.AddValuation({doe}, false);
		s0.AddFluent(enrolled);

		scs::Formula enrolled_ssa_form = BinaryConnective(a_eq(register_act), UnaryConnective(a_eq(unregister_act), UnaryKind::Negation), BinaryKind::Conjunction);
		scs::Successor enrolled_ssa{std::vector<scs::Term>{scs::Variable{"st"}}, enrolled_ssa_form}; // Contains terms that will be substituted in for unification and formula
		
		bat.successors["enrolled"] = enrolled_ssa;
		bat.SetInitial(s0);

	}
};

TEST_F(DatabaseSsaTest, EnrollDefaultValues) {
	ASSERT_EQ(enrolled.Valuation({ john }), false);
	ASSERT_EQ(enrolled.Valuation({ doe }), false);
}

TEST_F(DatabaseSsaTest, Enroll_Register_Doe) {
	EnableTracing();
	scs::Situation s_prime = s0.Do(scs::Action{"register", std::vector<Term>{Object{"Doe"}}}, bat);
	// EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"John"}}), false);
	// EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"Doe"}}), true);
	std::cout << "**************** \n";
}

