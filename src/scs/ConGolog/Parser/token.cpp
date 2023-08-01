#include "scs/ConGolog/Parser/token.h"

#include <string_view>
#include <optional>
#include <sstream>
#include <ostream>

#include <magic_enum.hpp>

namespace scs {

	Token::Token()
	: length(2703), line(2703) {}

	Token::Token(TokenType type, std::string_view view, size_t length, size_t line)
	: type(type), view(view), length(length), line(line) {}

	bool Token::operator==(const Token& other) const {
		if (type == other.type && view == other.view && length == other.length && line == other.line) {
			return true;
		}
		return false;
	}

	std::ostream& operator<<(std::ostream& os, const Token& token) {
		os << magic_enum::enum_name(token.type);
		os << " = ";
		os << token.view;
		return os;
	}

	std::string Token::ToStr() const {
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

	/* 
	 * @brief: Converts a raw string to a token 
	 */
	std::optional<TokenType> Token::StringToToken(std::string_view str) {
		auto token = 
			magic_enum::enum_cast<TokenType>(str, [](char lhs, char rhs) { return std::tolower(lhs) == std::tolower(rhs); });
		if (token.has_value()) {
			return {*token};
		}
		return std::nullopt;
	}

	/*
	 * @brief: Converts a raw string to a token of type keyword by assuming all keyword tokens >= 100 && <= 200
	 */
	std::optional<TokenType> Token::StringToKeyword(std::string_view str) {
		auto token = StringToToken(str);
		if (!token.has_value()) {
			return { std::nullopt };
		}
		// Assume all keywords are in range 100 to 200
		size_t val = static_cast<int>(*token);
		if (val >= 100 && val <= 200) {
			return { *token };
		}
		return std::nullopt;
	}

}