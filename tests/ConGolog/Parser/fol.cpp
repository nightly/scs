#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/fol.h"
#include "scs/ConGolog/Parser/lexer.h"
#include "scs/SituationCalculus/Parser/parser.h"

#include <string>
#include <vector>

TEST(FolParser, Primary) {
	std::string str1 = "true";
	std::string str2 = "True";
	scs::FolParser fol_parser_1(str1);
	ASSERT_EQ(std::get<bool>(fol_parser_1.ParseFormula()), true);
	scs::FolParser fol_parser_2(str2);
	ASSERT_EQ(std::get<bool>(fol_parser_2.ParseFormula()), true);

	std::string str3 = "     false     \n";
	std::string str4 = "FALSE";
	scs::FolParser fol_parser_3(str3);
	ASSERT_EQ(std::get<bool>(fol_parser_3.ParseFormula()), false);
	scs::FolParser fol_parser_4(str4);
	ASSERT_EQ(std::get<bool>(fol_parser_4.ParseFormula()), false);
}

TEST(FolParser, Conjunction) {
	scs::Formula f = scs::ParseScFormula("true and false");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), false);
}

TEST(FolParser, Disjunction) {
	scs::Formula f = scs::ParseScFormula("true or false");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, NegationPrimary) {
	scs::Formula f = scs::ParseScFormula("not false");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, NegationFormula) {
	scs::Formula f = scs::ParseScFormula("not (true and false)");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, Quantification) {
	scs::BasicActionTheory bat = scs::ParseBasicActionTheory(R"(
objects x
init ready(x) = true
)");
	scs::Formula f = scs::ParseScFormula("forall v. ready(v)");
	scs::Evaluator eval{{bat.Initial(), bat}};
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, NestedQuantification){
	scs::BasicActionTheory bat = scs::ParseBasicActionTheory(R"(
objects x, y
init ready(x) = true
init ready(y) = true
)");
	scs::Formula f = scs::ParseScFormula("forall v,w. ready(v) and ready(w)");
	scs::Evaluator eval{{bat.Initial(), bat}};
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, Parentheses) {
	scs::Formula f = scs::ParseScFormula("(false or true) and true");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, Precedence) {
	scs::Formula f = scs::ParseScFormula("false or true and false");
	scs::Evaluator eval;
	ASSERT_EQ(std::visit(eval, f), false);
}

TEST(FolParser, RelationalFluent) {
	scs::BasicActionTheory bat = scs::ParseBasicActionTheory(R"(
objects x
init ready(x) = true
)");
	scs::Formula f = scs::ParseScFormula("ready(obj(x))");
	scs::Evaluator eval{{bat.Initial(), bat}};
	ASSERT_EQ(std::visit(eval, f), true);
}

TEST(FolParser, Action) {
	scs::FirstOrderAssignment assignment;
	assignment.Set(scs::Variable{"a"}, scs::Action{"Load", {scs::Object{"tube"}}});
	scs::Formula f = scs::ParseScFormula("a = Load(obj(tube))");
	scs::Evaluator eval{assignment};
	ASSERT_EQ(std::visit(eval, f), true);
}
