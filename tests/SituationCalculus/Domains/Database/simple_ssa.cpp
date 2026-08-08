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

namespace {

	Formula Exists(const Variable& variable, const Formula& child) {
		return Box<Quantifier>{new Quantifier{variable, child, QuantifierKind::Existential}};
	}

	Formula SuccessorFormula(const Formula& positive_effect, const Action& removal) {
		return BinaryConnective{
			positive_effect,
			BinaryConnective{cv(), UnaryConnective{a_eq(removal), UnaryKind::Negation}, BinaryKind::Conjunction},
			BinaryKind::Disjunction};
	}

	BasicActionTheory FreshPartBat() {
		BasicActionTheory bat;
		bat.objects.emplace("brass");
		bat.objects.emplace("r2");

		Situation initial;
		initial.AddFluent("Part", RelationalFluent{1});
		initial.AddFluent("Material", RelationalFluent{2});
		initial.AddFluent("At", RelationalFluent{2});
		bat.SetInitial(initial);

		const Action admit{"Admit", {Variable{"x"}, Variable{"m"}, Variable{"r"}}};
		const Action store{"Store", {Variable{"x"}}};
		bat.pre["Admit"] = Poss{{Variable{"x"}, Variable{"m"}, Variable{"r"}}, true};
		bat.pre["Store"] = Poss{{Variable{"x"}}, true};

		bat.successors["Part"] = Successor{{Variable{"x"}},
			SuccessorFormula(Exists(Variable{"m"}, Exists(Variable{"r"}, a_eq(admit))), store)};
		bat.successors["Material"] = Successor{{Variable{"x"}, Variable{"m"}},
			SuccessorFormula(Exists(Variable{"r"}, a_eq(admit)), store)};
		bat.successors["At"] = Successor{{Variable{"x"}, Variable{"r"}},
			SuccessorFormula(Exists(Variable{"m"}, a_eq(admit)), store)};
		return bat;
	}

}

TEST(SparseProgression, FreshActionIdentifierCreatesTrueTuples) {
	auto bat = FreshPartBat();
	const Action admit{"Admit", {Object{"part_93842"}, Object{"brass"}, Object{"r2"}}};

	ASSERT_TRUE(bat.Initial().Possible(admit, bat));
	const Situation next = bat.Initial().Do(admit, bat, true);

	EXPECT_TRUE(next.Fluents().at("Part").Valuation({Object{"part_93842"}}));
	EXPECT_TRUE(next.Fluents().at("Material").Valuation({Object{"part_93842"}, Object{"brass"}}));
	EXPECT_TRUE(next.Fluents().at("At").Valuation({Object{"part_93842"}, Object{"r2"}}));
	EXPECT_FALSE(bat.objects.contains(Object{"part_93842"}));
}

TEST(SparseProgression, RemovalDeletesEveryTupleContainingTheIdentifier) {
	auto bat = FreshPartBat();
	const Action admit{"Admit", {Object{"part_93842"}, Object{"brass"}, Object{"r2"}}};
	const Situation admitted = bat.Initial().Do(admit, bat, true);
	const Situation stored = admitted.Do(Action{"Store", {Object{"part_93842"}}}, bat, true);

	EXPECT_TRUE(stored.Fluents().at("Part").TrueTuples().empty());
	EXPECT_TRUE(stored.Fluents().at("Material").TrueTuples().empty());
	EXPECT_TRUE(stored.Fluents().at("At").TrueTuples().empty());
}

TEST(SparseProgression, CompoundActionProgressesAllEffectsTogether) {
	auto bat = FreshPartBat();
	const CompoundAction action{{
		Action{"Admit", {Object{"part_93842"}, Object{"brass"}, Object{"r2"}}},
		Action{"Nop"}}};
	const Situation next = bat.Initial().Do(action, bat, true);

	EXPECT_TRUE(next.Fluents().at("Part").Valuation({Object{"part_93842"}}));
	EXPECT_TRUE(next.Fluents().at("Material").Valuation({Object{"part_93842"}, Object{"brass"}}));
	EXPECT_TRUE(next.Fluents().at("At").Valuation({Object{"part_93842"}, Object{"r2"}}));
}
