#include <gtest/gtest.h>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/windows.h"

using namespace scs;

class FolPredicateTest : public ::testing::Test {
protected:
	scs::Situation s;
	scs::BasicActionTheory bat;
	scs::Evaluator eval{{s, bat}};

	void SetUp() override {
		scs::SetConsoleEncoding();
		bat.objects.emplace("robot1");
		bat.objects.emplace("robot2");
		scs::RelationalFluent Holding{ "Holding" }, Safe{ "Safe" }, Loaded{"Loaded"}, Off{"Off"}, On{"On"};
		Holding.AddValuation({ "robot1" }, false);
		Holding.AddValuation({ "robot2" }, true);
		Safe.AddValuation({ "robot1" }, true);
		Safe.AddValuation({ "robot2" }, true);
		// 0-arity
		Off.AddValuation(false);
		On.AddValuation(true);

		s.AddFluent(Holding);
		s.AddFluent(Safe);
		s.AddFluent(Off);
		s.AddFluent(On);
		s.AddFluent("Loaded");
	}

};

TEST(FolPredicate, Create) {
	Term t1 = Object{ "o1" };
	Term t2 = Variable{ "v1" };
	Predicate p("p", { t1, t2 });
}


TEST(FolPredicate, Print) {
	std::ostringstream output;

	Term t1 = Object{ "o1" };
	Term t2 = Variable{ "v1" };
	Predicate p("p", { t1, t2 });
	
	output << p;
	EXPECT_EQ(output.str(), "p(obj(o1), var(v1))");
}

TEST_F(FolPredicateTest, BooleanIdentity) {
	scs::Formula f = true;
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = false;
	result = std::visit(eval, f);
	EXPECT_EQ(result, false);
}
TEST_F(FolPredicateTest, RelationFluent0Arity) {
	scs::Formula f = Predicate("Off");
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, false);

	f = Predicate("On");
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST_F(FolPredicateTest, RelationalFluentParams) {
	scs::Formula f = Predicate("Holding", std::vector<Term>{scs::Object{ "robot1" }});
	auto result = std::visit(eval, f);
	EXPECT_EQ(result, false);

	f = Predicate("Holding", std::vector<Term>{scs::Object{ "robot2" }});
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = Predicate("Safe", std::vector<Term>{scs::Object{ "robot1" }});
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);

	f = Predicate("Safe", std::vector<Term>{scs::Object{ "robot2" }});
	result = std::visit(eval, f);
	EXPECT_EQ(result, true);
}

TEST_F(FolPredicateTest, RelationalFluentMissingValuation) {
	Formula f = Predicate("Loaded", std::vector<Term>{scs::Object{ "robot3" }, scs::Object("robot4")});
	bool result = std::visit(eval, f);
	EXPECT_EQ(result, false);
}