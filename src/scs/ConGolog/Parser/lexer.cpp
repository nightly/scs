#include "scs/ConGolog/Parser/lexer.h"

#include <string>
#include <filesystem>
#include <stdexcept>
#include <format>
#include <iostream>

#include "magic_enum.hpp"

#include "scs/Common/log.h"
#include "scs/ConGolog/Parser/utf8.h"
#include "scs/Common/io.h"
#include "scs/Common/strings.h"
#include "scs/Common/windows.h"

namespace scs {


	Lexer::Lexer(const std::string& input)
		: source(input) {
		Init();
	}

	Lexer::Lexer(std::string_view source)
		: source(source) {
		Init();
	}

	Lexer::Lexer(const char* source)
		: source(source) {
		Init();
	}

	Lexer::Lexer(std::string&& source)
		: source(std::move(source)) {
		Init();
	}

	Lexer::Lexer(const std::filesystem::path& path) {
		source = scs::ReadIntoString(path);
		Init();
	}

	void Lexer::Init() {
		Reset();
		source += '\n';
		source_size = source.length();
	}

	void Lexer::Reset() {
		current_line = 1;
		current_pos = -1;
		current_char = &space_char;
	}

	void Lexer::SkipWhitespace() {
		while (*current_char.data() == ' ' || *current_char.data() == '\t' || *current_char.data() == '\r') {
			NextChar();
		}
	}

	void Lexer::SkipComment() {
		if (*current_char.data() == '%') {
			while (*current_char.data() != '\n') {
				NextChar();
			}
		}
	}

	/**
	 * @brief: Extracts the next 'char' — but with multibyte utf8 support.
	 * @param: pos: starting position
	 * @returns: the string_view of the next character, and the number of bytes moved forward
	 */
	std::pair<std::string_view, size_t> Lexer::ExtractChar(size_t pos) {
		std::string_view view;
		if (pos + 1 >= source_size) {
			return {&null_char, 1};
		}

		size_t bytes = NumberOfBytes(&source[pos]);
		view = std::string_view(source.c_str() + pos, bytes);

		return {view, bytes};
	}

	/** 
	 * @returns: The number of bytes the current character occupies (for advancing multibyte utf8).
	 * This is useful to maintain the current_pos but this function could automatically take care of it anyway.
	 */
	void Lexer::NextChar() {
		if (*current_char.data() == '\n') {
			current_line++;
			line_start_pos__ = current_pos;
		}
		current_pos += 1;
		const auto& [view, bytes] = ExtractChar(current_pos);
		current_char = view;
		current_pos += (bytes - 1); // Handle multibyte advances knowing we already moved forward one byte
	}

	std::string_view Lexer::Peek() {
		return ExtractChar(current_pos + 1).first;
	}

	void Lexer::Abort(const std::string& message) {
		std::cerr << message << std::endl;
		std::exit(1);
	}

	Token Lexer::MakeToken(TokenType type) {
		return Token(type, current_char, current_char.size(), current_line);
	}

	Token Lexer::MakeToken(TokenType type, std::string_view view) {
		return Token(type, view, view.size(), current_line);
	}

	Token Lexer::NextToken() {
		SkipComment();
		SkipWhitespace();
		Token token;
		std::string_view& view = current_char;

		/************************ 1 character tokens **************************/
		if (*view.data() == '\0') {
			token = MakeToken(TokenType::EndOfFile);
		} else if (view == "\n") {
			token = MakeToken(TokenType::Newline);
		} else if (view == "#") {
			NextChar();
			size_t len = 1;
			size_t start = current_pos;
			while (std::isalnum(*Peek().data(), loc) && (!IsBlank(*Peek().data())) && (*Peek().data() != '\n') && (*Peek().data() != '\0')) {
				NextChar();
				len++;
			}
			// std::string_view substr(source.c_str() + start, source.c_str() + (current_pos));
			std::string substr = source.substr(start, len);
			substr = scs::ToLower(substr);
			if (substr == "bat") {
				token = MakeToken(TokenType::DirectiveBAT, "#BAT");
			} else if (substr == "program") {
				token = MakeToken(TokenType::DirectiveProgram, "#Program");
			} else {
				Abort(std::format("[SCS Lexer] Error parsing {}, expected a directive of Program or BAT.", substr));
			}
		} else if (view == "⊃") {
			token = MakeToken(TokenType::Implies);
		} else if (view == "≡") {
			token = MakeToken(TokenType::Equivalence);
		} else if (view == "∧" || view == "^") {
			token = MakeToken(TokenType::And);
		} else if (view == "∨") {
			token = MakeToken(TokenType::Or);
		} else if (view == "¬") {
			token = MakeToken(TokenType::Negation);
		} else if (view == "∀") {
			token = MakeToken(TokenType::Universal);
		} else if (view == "∃") {
			token = MakeToken(TokenType::Existential);
		} else if (view == ":") {
			token = MakeToken(TokenType::Colon);
		} else if (view == "=") {
			token = MakeToken(TokenType::Equal);
		} else if (view == "(") {
			token = MakeToken(TokenType::LParen);
		} else if (view == ")") {
			token = MakeToken(TokenType::RParen);
		} else if (view == ",") {
			token = MakeToken(TokenType::Comma);
		} else if (view == ";") {
			token = MakeToken(TokenType::SemiColon);
		} else if (view == ".") {
			token = MakeToken(TokenType::Dot);
		} else if (view == "*") {
			token = MakeToken(TokenType::Star);
		} else if (view == "|") {
			if (Peek() == "|") {
				NextToken();
				if (Peek() == "|") {
					NextToken();
					token = MakeToken(TokenType::InterleavedConcurrency, "|||");
				} else {
					token = MakeToken(TokenType::SynchronizedConcurrency, "||");
				}
			} else {
				token = MakeToken(TokenType::NonDet);
			}
		} else if (std::isalpha(*view.data())) { // Identifier/Keyword
			size_t start = current_pos;
			while (std::isalnum(*Peek().data(), loc)) {
				NextChar();
			}
			std::string_view str(source.c_str() + start, source.c_str() + (current_pos + 1));

			// Disambiguate between identifiers and keywords:
			auto keyword = scs::Token::StringToKeyword(str);
			if (keyword.has_value()) {
				token = MakeToken(*keyword, str);
			} else {
				token = MakeToken(TokenType::Identifier, str);
			}
		} else if (std::isdigit(*view.data())) {
			size_t start = current_pos;
			while (std::isdigit(*Peek().data(), loc) || (*Peek().data() == '.')) { // should only allow 1 decimal tho
				NextChar();
			}
			std::string_view str(source.c_str() + start, source.c_str() + (current_pos + 1));
			token = MakeToken(TokenType::Number, str);
		} else {
			Abort(std::format("[SCS Lexer] Unknown token '{}' at line {} at relative line position {}", current_char,
				current_line, (current_pos - line_start_pos__)));
		}

		NextChar();
		return token;
	}

	/**
	 * @brief: Grabs all tokens from the starting position
	 */
	std::vector<Token> Lexer::AllTokens() {
		auto old_pos = current_pos;
		auto old_char = current_char;
		std::vector<Token> ret;

		while (*Peek().data() != '\0') {
			ret.emplace_back(NextToken());
		}
		ret.emplace_back(NextToken());

		current_pos = old_pos;
		current_char = old_char;
		return ret;
	}

	std::ostream& operator<<(std::ostream& os, Lexer& lexer) {
		scs::SetConsoleEncoding();

		os << "============== [SCS Lexer Stream] Starting lexing " << "============== \n";
		std::vector<Token> tokens = lexer.AllTokens();
		for (const auto& token : tokens) {
			os << token << '\n';
		}
		os << "============== [SCS Lexer Stream] Ending lexing " << "============== \n";
		return os;
	}
}
