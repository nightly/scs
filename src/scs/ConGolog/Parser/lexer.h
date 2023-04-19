#pragma once

#pragma execution_character_set("utf-8")

#include <string>
#include <filesystem>
#include <string_view>
#include <vector>

#include "scs/ConGolog/Parser/token.h"

namespace scs {

	enum class LexerError {
		Generic,
	};

	class Lexer {
	public:
		static constexpr char space_char = ' ';
		static constexpr char null_char = '\0';
		std::locale loc{};
	private:
		std::string source;
		size_t source_size;
		std::string_view current_char;
		int64_t current_pos;
		size_t current_line;
		size_t line_start_pos__ = 0;
	public:
		Lexer(const std::string& source);
		Lexer(std::string_view source);
		Lexer(const char* source);
		Lexer(std::string&& source);
		Lexer(const std::filesystem::path& path);
		Token NextToken();
		std::vector<Token> AllTokens();

		void Reset();

		friend std::ostream& operator<<(std::ostream& os, Lexer& lexer);
	private:
		void Init();

		std::pair<std::string_view, size_t> ExtractChar(size_t pos);
		void NextChar();
		std::string_view Peek();

		Token MakeToken(TokenType type);
		Token MakeToken(TokenType type, std::string_view view);
		
		void SkipWhitespace();
		void SkipComment();
		
		void Abort(const std::string& message);
	};

}