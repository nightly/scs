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
	scs::Action register_act{"register", {Variable{"x"}}};
	scs::Action unregister_act{"unregister", {Variable{"x"}}};

	scs::Object doe{"Doe", s0};
	scs::Object john{"John", s0};

	void SetUp() override {
		scs::SetConsoleEncoding();
		enrolled.AddValuation({ john }, true);
		enrolled.AddValuation({doe}, false);
		s0.AddFluent(enrolled);

		scs::Formula enrolled_ssa_form = BinaryConnective(a_eq(register_act), UnaryConnective(a_eq(unregister_act), UnaryKind::Negation), BinaryKind::Conjunction);
		scs::Successor enrolled_ssa{s0.relational_fluents_["enrolled"], enrolled_ssa_form};
		
		bat.successors["enrolled"] = enrolled_ssa;
		bat.SetInitial(s0);

	}
};

TEST_F(DatabaseSsaTest, EnrollDefaultValues) {
	ASSERT_EQ(enrolled.Valuation({ john }), true);
	ASSERT_EQ(enrolled.Valuation({ doe }), false);
}

TEST_F(DatabaseSsaTest, Enroll_Register) {
	// s0.PrintFluents();
	std::cout << "========= \n";
	std::cout << bat.successors["enrolled"].Form() << std::endl;
	std::cout << "========= \n";

}

TEST_F(DatabaseSsaTest, Enroll_Unregister) {

}