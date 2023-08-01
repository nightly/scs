#include <gtest/gtest.h>

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/FirstOrderLogic/fol.h"

#include "scs/Common/windows.h"

using namespace scs;

class DatabaseSsaTest : public ::testing::Test {
protected:
	scs::BasicActionTheory bat;
	scs::Situation s0;
	scs::RelationalFluent enrolled;
	scs::Action register_act{"register", {Variable{"st"}}}; // Abstract action type for SSA
	scs::Action unregister_act{"unregister", {Variable{"st"}}}; // Abstract action type for SSA

	scs::Object doe{"Doe", bat};
	scs::Object john{"John", bat};

	void SetUp() override {
		scs::SetConsoleEncoding();
		enrolled.AddValuation({ john }, true);
		enrolled.AddValuation({doe}, false);
		s0.AddFluent("enrolled", enrolled);

		scs::Formula enrolled_ssa_form = BinaryConnective(BinaryConnective(scs::Variable{"cv"}, a_eq(register_act), BinaryKind::Disjunction), 
			UnaryConnective(a_eq(unregister_act), UnaryKind::Negation), BinaryKind::Conjunction);
		scs::Successor enrolled_ssa{std::vector<scs::Term>{scs::Variable{"st"}}, enrolled_ssa_form};

		bat.successors["enrolled"] = enrolled_ssa;
		bat.SetInitial(s0);

	}
};

TEST_F(DatabaseSsaTest, EnrollDefaultValues) {
	ASSERT_EQ(enrolled.Valuation({ john }), true);
	ASSERT_EQ(enrolled.Valuation({ doe }), false);
}

TEST_F(DatabaseSsaTest, EnrollRegisterDoe) {
	// LogModeTracing();
	scs::Situation s_prime = s0.Do(scs::Action{"register", std::vector<Term>{Object{"Doe"}}}, bat);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"John"}}), true);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"Doe"}}), true);
	std::cout << "**************** \n";
}

TEST_F(DatabaseSsaTest, EnrollCompoundRegisterDoe) {
	Action a1 = scs::Action{ "register", std::vector<Term>{Object{"Doe"}}};
	Action a2 = scs::Action{ "Nop", std::vector<Term>{}};
	CompoundAction ca({a1, a2});
	
	scs::Situation s_prime = s0.Do(ca, bat);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"John"}}), true);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({Object{"Doe"}}), true);
	std::cout << "**************** \n";
}

TEST_F(DatabaseSsaTest, Enroll_Unregister_Doe) {
	scs::Situation s_prime = s0.Do(scs::Action{"unregister", std::vector<Term>{Object{"John"}}}, bat);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({ Object{"John"} }), false);
	EXPECT_EQ(s_prime.relational_fluents_.at("enrolled").Valuation({ Object{"Doe"} }), false);
}
