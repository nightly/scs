#include <gtest/gtest.h>

#include "scs/SituationCalculus/Parser/parser.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/Synthesis/Exact/model.h"

using namespace scs;

TEST(ScParser, BooleanAndPrecedence) {
	Formula f = ParseScFormula("false or true and false");
	Evaluator eval;
	EXPECT_EQ(std::visit(eval, f), false);
}

TEST(ScParser, UnicodeAndAsciiAliases) {
	Formula f = ParseScFormula("not false ∧ true && (false or true)");
	Evaluator eval;
	EXPECT_EQ(std::visit(eval, f), true);
}

TEST(ScParser, UniversalMultiVariable) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects robot1, robot2
init Safe(robot1) = true
init Safe(robot2) = true
)");

	Formula f = ParseScFormula("forall x,y. Safe(x) and Safe(y)");
	Evaluator eval{{bat.Initial(), bat}};
	EXPECT_EQ(std::visit(eval, f), true);
}

TEST(ScParser, FalseInitialRowsRemainAbsentFromSparseExtensions) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects robot, plate, cup
init holding(robot, plate) = false
init holding(robot, cup) = true
)");

	const auto& holding = bat.Initial().Fluents().at("holding");
	ASSERT_EQ(holding.TrueTuples().size(), 1);
	EXPECT_FALSE(holding.Valuation({Object{"robot"}, Object{"plate"}}));
	EXPECT_TRUE(holding.Valuation({Object{"robot"}, Object{"cup"}}));
}

TEST(ScParser, ExplicitTermsAndActionEquality) {
	FirstOrderAssignment assignment;
	assignment.Set(Variable{"a"}, Action{"Load", {Object{"tube"}, Object{"2"}}});

	Formula f = ParseScFormula("a = Load(obj(tube), obj(2))");
	Evaluator eval{assignment};
	EXPECT_EQ(std::visit(eval, f), true);
}

TEST(ScParser, DeclaredObjectsResolveInsidePredicateTerms) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects brass, 2
init at(brass, 2) = true
poss Check = at(brass, 2)
)");

	const auto& formula = bat.pre.at("Check").Form();
	const auto& pred = std::get<Predicate>(formula);
	ASSERT_EQ(pred.terms().size(), 2);
	EXPECT_EQ(std::get<Object>(pred.terms()[0]), Object{"brass"});
	EXPECT_EQ(std::get<Object>(pred.terms()[1]), Object{"2"});

	Action check{"Check"};
	EXPECT_TRUE(bat.Initial().Possible(check, bat));
}

TEST(ScParser, DeclaredObjectsResolveInsideActionTerms) {
	ParseContext context;
	context.objects.insert("brass");
	context.objects.insert("2");
	context.variables.insert("a");

	Formula f = ParseScFormula("a = Load(brass, 2)", context);
	FirstOrderAssignment assignment;
	assignment.Set(Variable{"a"}, Action{"Load", {Object{"brass"}, Object{"2"}}});

	Evaluator eval{assignment};
	EXPECT_TRUE(std::visit(eval, f));
}

TEST(ScParser, FormalVariablesCannotShadowObjects) {
	EXPECT_THROW(ParseBasicActionTheory(R"(
objects brass
poss Bad(brass) = true
)"), std::runtime_error);
}

TEST(ScParser, GlobalValidationCatchesMissingFormulaObject) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects tube
init ready(tube) = true
poss Check = ready(obj(brass))
)", ScParseOptions{ValidationMode::Local, true});

	EXPECT_THROW(ValidateBasicActionTheory(bat, ValidationMode::Global), std::runtime_error);
}

TEST(ScParser, ParsesBasicActionTheory) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects tube, 2
type Load = manufacturing
init part(tube) = true
init on_site(tube) = true
poss Load(part, i) = part(part) and on_site(part)
)");

	EXPECT_TRUE(bat.objects.contains(Object{"tube"}));
	EXPECT_TRUE(bat.objects.contains(Object{"2"}));
	EXPECT_TRUE(bat.pre.contains("Load"));

	Action load{"Load", {Object{"tube", bat}, Object{"2", bat}}};
	EXPECT_TRUE(bat.Initial().Possible(load, bat));
}

TEST(ScParser, GlobalValidationCatchesMissingInitialFluent) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects x
type Act = manufacturing
init known(x) = true
poss Act(v) = missing(v)
)", ScParseOptions{ValidationMode::Local, true});

	EXPECT_THROW(ValidateBasicActionTheory(bat, ValidationMode::Global), std::runtime_error);
}

TEST(ScParser, MergedValidationAllowsCrossResourceFluents) {
	BasicActionTheory common = ParseBasicActionTheory(R"(
objects item
init ready(item) = true
ssa ready(x) = cv
)");
	BasicActionTheory resource = ParseBasicActionTheory(R"(
objects machine
type Use = manufacturing
poss Use(x) = ready(x)
)");

	FacilityComposition composition;
	composition.common = std::move(common);
	composition.callbacks.observe = [](const JointAction&) { return std::optional<CompoundAction>{}; };
	EXPECT_NO_THROW(ComposeFacility({Resource{1, std::make_shared<Nil>(), std::move(resource)}},
		std::move(composition)));
}

TEST(ScParser, RigidRelationsAreOrdinaryDeclaredPredicates) {
	BasicActionTheory bat = ParseBasicActionTheory(R"(
objects r1, r2
rigid Route(r1, r2) = true
rigid Route(r2, r1) = false
poss Move = Route(r1, r2)
)");
	EXPECT_TRUE(bat.rigid_objects.contains(Object::Rigid("r1")));
	EXPECT_TRUE(bat.rigid.at("Route").Valuation({Object::Rigid("r1"), Object::Rigid("r2")}));
	EXPECT_EQ(bat.rigid.at("Route").ExplicitValuation({Object::Rigid("r2"), Object::Rigid("r1")}), false);
	EXPECT_TRUE(bat.Initial().Possible(Action{"Move"}, bat));
	EXPECT_NO_THROW(ValidateBasicActionTheory(bat, ValidationMode::Global));
	EXPECT_THROW(ParseBasicActionTheory(R"(
objects r1, r2
rigid Route(r1, r2) = true
rigid Route(r1, r2) = false
)"), std::runtime_error);
}
