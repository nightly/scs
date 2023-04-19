#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/lexer.h"

#include <string>
#include <vector>

#include "scs/Common/io.h"

TEST(Lexer, Utf8) {
	std::string tokens("  \n^ \n∀∧ \0");
	scs::Lexer lex(tokens);
	std::cout << lex;
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::Newline);
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::And);
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::Newline);
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::Universal);
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::And);
	ASSERT_EQ(lex.NextToken().type, scs::TokenType::EndOfFile);
}

TEST(Lexer, Whitespaces) {
	std::string program("     #Program     ");
	scs::Lexer lex(program);
	auto got = lex.NextToken().type;
	ASSERT_EQ(got, scs::TokenType::DirectiveProgram);
}

TEST(Lexer, Empty) {
	std::string program("");
	scs::Lexer lex(program);
	auto got = lex.NextToken().type;
	ASSERT_EQ(got, scs::TokenType::EndOfFile);
}

TEST(Lexer, Directive) {
	std::string prog("\n \n #BAT \n \n \n #Program \n\n \0");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[2].type, scs::TokenType::DirectiveBAT);
	ASSERT_EQ(tokens[6].type, scs::TokenType::DirectiveProgram);
	ASSERT_EQ(tokens[9].type, scs::TokenType::EndOfFile);
}

TEST(Lexer, DirectiveCasing) {
	std::string prog("\n \n #bat \n \n \n #ProGram \n\n \0");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[2].type, scs::TokenType::DirectiveBAT);
	ASSERT_EQ(tokens[6].type, scs::TokenType::DirectiveProgram);
	ASSERT_EQ(tokens[9].type, scs::TokenType::EndOfFile);
}

TEST(Lexer, Keyword) {
	std::string prog("\n \n #Program \n \n \n Do \n \n \0");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[2].type, scs::TokenType::DirectiveProgram);
	ASSERT_EQ(tokens[6].type, scs::TokenType::Do);
	ASSERT_EQ(tokens[6].view, "Do");
}

TEST(Lexer, Identifier) {
	std::string prog("var1\n");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[0].type, scs::TokenType::Identifier);
	ASSERT_EQ(tokens[0].view, "var1");
}

TEST(Lexer, Number) {
	std::string prog("30\n");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[0].type, scs::TokenType::Number);
	ASSERT_EQ(tokens[0].view, "30");
}

TEST(Lexer, Decimal) {
	std::string prog("0.30\n");
	scs::Lexer lex(prog);
	auto tokens = lex.AllTokens();
	ASSERT_EQ(tokens[0].type, scs::TokenType::Number);
	ASSERT_EQ(tokens[0].view, "0.30");
}

