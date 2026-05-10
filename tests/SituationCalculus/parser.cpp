#include <gtest/gtest.h>

#include "scs/SituationCalculus/Parser/parser.h"
#include "scs/Synthesis/global_bat.h"
#include "Hinge/common.h"
#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"

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
)");
	BasicActionTheory resource = ParseBasicActionTheory(R"(
objects machine
type Use = manufacturing
poss Use(x) = ready(x)
)");

	std::vector<BasicActionTheory> bats{common, resource};
	CoopMatrix coop{10};
	RoutesMatrix routes{10};
	EXPECT_NO_THROW(CombineBATs(bats, coop, routes));
}

TEST(ScParser, HingeParsedObjectDomainMatchesTranslatedFormulas) {
	auto common = examples::HingeCommonBAT();
	auto r1 = examples::HingeResource1();
	auto r2 = examples::HingeResource2();
	auto r3 = examples::HingeResource3();
	auto r4 = examples::HingeResource4();

	std::vector<BasicActionTheory> bats{common, r1.bat, r2.bat, r3.bat, r4.bat};
	CoopMatrix coop{10};
	RoutesMatrix routes{10};
	BasicActionTheory global = CombineBATs(bats, coop, routes);

	std::vector<Object> expected_objects{
		"brass", "tube", "5", "1", "2", "3", "4", "ok", "1.5", "3mm", "5mm", "metallic_red", "metallic_blue"
	};
	for (const Object& object : expected_objects) {
		EXPECT_TRUE(global.objects.contains(object)) << object.name();
	}

	EXPECT_TRUE(global.Initial().relational_fluents_.at("suitable").Valuation({Object{"5mm"}, Object{"1.5"}}));
	EXPECT_FALSE(global.Initial().relational_fluents_.at("clamped").Valuation({Object{"brass"}, Object{"5"}, Object{"1"}}));
	EXPECT_TRUE(global.Initial().relational_fluents_.at("safe_force").Valuation({Object{"tube"}, Object{"0.5"}}));
	EXPECT_FALSE(global.Initial().relational_fluents_.at("equipped_bit").Valuation({Object{"5mm"}, Object{"3"}}));

	EXPECT_NO_THROW(ValidateBasicActionTheory(global, ValidationMode::Global));
}
