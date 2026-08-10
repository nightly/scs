#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace scs {

	enum class TokenType {
		DirectiveBAT = 0,
		DirectiveProgram = 1,
		Identifier = 2,
		Number = 3,
		Newline = 4,
		EndOfFile = 5,
		Colon = 6,
		Dot = 7,
		And = 8,
		Or = 9,
		Negation = 10,
		Implies = 11,
		Equivalence = 12,
		Universal = 13,
		Existential = 14,
		Equal = 15,
		NotEqual = 16,
		LParen = 17,
		RParen = 18,
		Comma = 19,
		True = 101,
		False = 102,
		Do = 103,
		Loop = 104,
		While = 105,
		Endwhile = 106,
		If = 107,
		Then = 108,
		Else = 109,
		EndIf = 110,
		SemiColon = 111,
		QuestionMark = 112,
		Star = 113,
		Pi = 114,
		Nil = 115,
		EndLoop = 116,
		NonDet = 130,
		InterleavedConcurrency = 131,
		SynchronizedConcurrency = 132,
		Poss = 201,
		S0 = 202,
		Obj = 301,
		Var = 302,
	};

	struct Token {
		TokenType type = TokenType::EndOfFile;
		std::string view;
		size_t length = 0;
		size_t line = 1;
		size_t column = 1;
		size_t offset = 0;

		Token() = default;
		Token(TokenType type, std::string_view view, size_t length, size_t line,
			size_t column = 1, size_t offset = 0);

		static std::optional<TokenType> StringToToken(std::string_view str);
		static std::optional<TokenType> StringToKeyword(std::string_view str);

		friend std::ostream& operator<<(std::ostream& os, const Token& token);
		std::string ToStr() const;
		bool operator==(const Token& other) const = default;
	};

}
