#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/token.h"

#include <string>

TEST(Token, Token) {
	std::string keyword = "Universal";
	ASSERT_EQ(scs::Token::StringToToken(keyword).has_value(), true);
}

TEST(Token, TokenCasing) {
	std::string keyword = "imPlies";
	ASSERT_EQ(scs::Token::StringToToken(keyword).has_value(), true);
}

TEST(Token, Keyword) {
	std::string universal = "Universal";
	ASSERT_EQ(scs::Token::StringToKeyword(universal).has_value(), false);

	std::string keyword_while = "While";
	ASSERT_EQ(scs::Token::StringToKeyword(keyword_while).has_value(), true);

	std::string keyword_if = "If";
	ASSERT_EQ(scs::Token::StringToKeyword(keyword_if).has_value(), true);
}