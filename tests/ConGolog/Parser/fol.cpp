#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/fol.h"
#include "scs/ConGolog/Parser/lexer.h"

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

}

TEST(FolParser, Disjunction) {

}

TEST(FolParser, NegationPrimary) {

}

TEST(FolParser, NegationFormula) {

}

TEST(FolParser, Quantification) {

}

TEST(FolParser, NestedQuantification){

}

TEST(FolParser, Parentheses) {

}

TEST(FolParser, Precedence) {

}

TEST(FolParser, RelationalFluent) {

}

TEST(FolParser, Action) {

}