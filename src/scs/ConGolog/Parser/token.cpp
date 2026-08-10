#include "scs/ConGolog/Parser/token.h"

#include <cctype>
#include <ostream>
#include <sstream>

#include <magic_enum/magic_enum.hpp>

namespace scs {

	Token::Token(TokenType type, std::string_view view, size_t length, size_t line,
		size_t column, size_t offset)
		: type(type), view(view), length(length), line(line), column(column), offset(offset) {}

	std::ostream& operator<<(std::ostream& os, const Token& token) {
		return os << magic_enum::enum_name(token.type) << " = " << token.view;
	}

	std::string Token::ToStr() const {
		std::stringstream stream;
		stream << *this;
		return stream.str();
	}

	std::optional<TokenType> Token::StringToToken(std::string_view str) {
		return magic_enum::enum_cast<TokenType>(str, [](char lhs, char rhs) {
			return std::tolower(static_cast<unsigned char>(lhs)) ==
				std::tolower(static_cast<unsigned char>(rhs));
		});
	}

	std::optional<TokenType> Token::StringToKeyword(std::string_view str) {
		auto token = StringToToken(str);
		if (!token) {
			return std::nullopt;
		}
		const auto value = static_cast<int>(*token);
		return value >= 100 && value <= 200 ? token : std::nullopt;
	}

}
