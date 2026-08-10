#pragma once

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#include "scs/ConGolog/Parser/token.h"

namespace scs {

	class Lexer {
	public:
		explicit Lexer(const std::string& source);
		explicit Lexer(std::string_view source);
		explicit Lexer(const char* source);
		explicit Lexer(std::string&& source);
		explicit Lexer(const std::filesystem::path& path);

		Token NextToken();
		std::vector<Token> AllTokens();
		void Reset();
		const std::string& Source() const noexcept { return source_; }

		friend std::ostream& operator<<(std::ostream& os, Lexer& lexer);
	private:
		std::string source_;
		size_t position_ = 0;
		size_t line_ = 1;
		size_t column_ = 1;

		bool StartsWith(std::string_view value) const;
		void Advance(size_t bytes = 1);
		void SkipHorizontalWhitespaceAndComments();
		Token Make(TokenType type, size_t begin, size_t line, size_t column) const;
		Token ScanIdentifier();
		Token ScanNumber();
		Token ScanDirective();
		[[noreturn]] void Fail(std::string_view message, size_t line, size_t column) const;
	};

}
