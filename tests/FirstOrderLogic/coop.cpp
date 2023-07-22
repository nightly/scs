#include <gtest/gtest.h>
#include "scs/SituationCalculus/coop_matrix.h"
#include "scs/FirstOrderLogic/fol.h"

TEST(ScCoopMatrix, Empty) {
	scs::CoopMatrix cm{5};
	scs::Domain d;
	d.mat = &cm;
	scs::Formula coop_pred = scs::CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
	scs::FirstOrderAssignment assignment;

	assignment.Set("i", scs::Object{"10"});
	assignment.Set("j", scs::Object{"10"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);

	assignment.Set("i", scs::Object{"0"});
	assignment.Set("j", scs::Object{"0"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);

	assignment.Set("i", scs::Object{"1"});
	assignment.Set("j", scs::Object{"1"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);
}

TEST(ScCoopMatrix, False) {
	scs::CoopMatrix cm{ 5 };
	cm.Add(0, 1);
	scs::Domain d;
	d.mat = &cm;
	scs::Formula coop_pred = scs::CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
	scs::FirstOrderAssignment assignment;
	scs::Evaluator eval(d, assignment);

	assignment.Set("i", scs::Object{"10"});
	assignment.Set("j", scs::Object{"10"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);

	assignment.Set("i", scs::Object{"1"});
	assignment.Set("j", scs::Object{"2"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);
}

TEST(ScCoopMatrix, True) {
	scs::CoopMatrix cm{ 5 };
	cm.Add(0, 1);
	scs::Domain d;
	d.mat = &cm;
	scs::Formula coop_pred = scs::CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
	scs::FirstOrderAssignment assignment;
	scs::Evaluator eval(d, assignment);

	assignment.Set("i", scs::Object{"0"});
	assignment.Set("j", scs::Object{"1"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), true);

	assignment.Set("i", scs::Object{"2"});
	assignment.Set("j", scs::Object{"3"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), false);

	assignment.Set("i", scs::Object{"1"});
	assignment.Set("j", scs::Object{"0"});
	EXPECT_EQ(std::visit(scs::Evaluator(d, assignment), coop_pred), true);
}