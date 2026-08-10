#include "scs/ConGolog/Parser/lexer.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "scs/Common/io.h"

namespace scs {
namespace {

	std::string Lower(std::string_view value) {
		std::string result(value);
		std::ranges::transform(result, result.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});
		return result;
	}

	bool IsIdentifierStart(unsigned char c) {
		return std::isalpha(c) || c == '_';
	}

	bool IsIdentifierPart(unsigned char c) {
		return std::isalnum(c) || c == '_' || c == '\'';
	}

	std::optional<TokenType> Keyword(std::string_view word) {
		const std::string lower = Lower(word);
		if (lower == "true") return TokenType::True;
		if (lower == "false") return TokenType::False;
		if (lower == "do") return TokenType::Do;
		if (lower == "loop") return TokenType::Loop;
		if (lower == "endloop") return TokenType::EndLoop;
		if (lower == "while") return TokenType::While;
		if (lower == "endwhile") return TokenType::Endwhile;
		if (lower == "if") return TokenType::If;
		if (lower == "then") return TokenType::Then;
		if (lower == "else") return TokenType::Else;
		if (lower == "endif") return TokenType::EndIf;
		if (lower == "pick" || lower == "pi") return TokenType::Pi;
		if (lower == "nil") return TokenType::Nil;
		if (lower == "and") return TokenType::And;
		if (lower == "or") return TokenType::Or;
		if (lower == "not") return TokenType::Negation;
		if (lower == "implies") return TokenType::Implies;
		if (lower == "equiv") return TokenType::Equivalence;
		if (lower == "forall") return TokenType::Universal;
		if (lower == "exists") return TokenType::Existential;
		if (lower == "poss") return TokenType::Poss;
		if (lower == "s0") return TokenType::S0;
		if (lower == "obj") return TokenType::Obj;
		if (lower == "var") return TokenType::Var;
		return std::nullopt;
	}

}

	Lexer::Lexer(const std::string& source) : source_(source) {}
	Lexer::Lexer(std::string_view source) : source_(source) {}
	Lexer::Lexer(const char* source) : source_(source == nullptr ? "" : source) {}
	Lexer::Lexer(std::string&& source) : source_(std::move(source)) {}
	Lexer::Lexer(const std::filesystem::path& path) : source_(ReadIntoString(path)) {}

	void Lexer::Reset() {
		position_ = 0;
		line_ = 1;
		column_ = 1;
	}

	bool Lexer::StartsWith(std::string_view value) const {
		return position_ + value.size() <= source_.size() &&
			std::string_view(source_).substr(position_, value.size()) == value;
	}

	void Lexer::Advance(size_t bytes) {
		for (size_t i = 0; i < bytes && position_ < source_.size(); ++i) {
			if (source_[position_] == '\n') {
				++line_;
				column_ = 1;
			} else {
				++column_;
			}
			++position_;
		}
	}

	void Lexer::SkipHorizontalWhitespaceAndComments() {
		for (;;) {
			while (position_ < source_.size() &&
				(source_[position_] == ' ' || source_[position_] == '\t' || source_[position_] == '\r')) {
				Advance();
			}
			if ((position_ < source_.size() && source_[position_] == '%') || StartsWith("//")) {
				while (position_ < source_.size() && source_[position_] != '\n') {
					Advance();
				}
				continue;
			}
			break;
		}
	}

	Token Lexer::Make(TokenType type, size_t begin, size_t line, size_t column) const {
		return {type, std::string_view(source_).substr(begin, position_ - begin),
			position_ - begin, line, column, begin};
	}

	Token Lexer::ScanIdentifier() {
		const size_t begin = position_;
		const size_t line = line_;
		const size_t column = column_;
		while (position_ < source_.size() &&
			IsIdentifierPart(static_cast<unsigned char>(source_[position_]))) {
			Advance();
		}
		const std::string_view word(source_.data() + begin, position_ - begin);
		return Make(Keyword(word).value_or(TokenType::Identifier), begin, line, column);
	}

	Token Lexer::ScanNumber() {
		const size_t begin = position_;
		const size_t line = line_;
		const size_t column = column_;
		while (position_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position_]))) {
			Advance();
		}
		if (position_ + 1 < source_.size() && source_[position_] == '.' &&
			std::isdigit(static_cast<unsigned char>(source_[position_ + 1]))) {
			Advance();
			while (position_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position_]))) {
				Advance();
			}
		}
		return Make(TokenType::Number, begin, line, column);
	}

	Token Lexer::ScanDirective() {
		const size_t begin = position_;
		const size_t line = line_;
		const size_t column = column_;
		Advance();
		const size_t word_begin = position_;
		while (position_ < source_.size() &&
			IsIdentifierPart(static_cast<unsigned char>(source_[position_]))) {
			Advance();
		}
		const std::string word = Lower(std::string_view(source_).substr(word_begin, position_ - word_begin));
		if (word == "bat") return Make(TokenType::DirectiveBAT, begin, line, column);
		if (word == "program") return Make(TokenType::DirectiveProgram, begin, line, column);
		Fail(std::format("unknown directive '#{}'", word), line, column);
	}

	Token Lexer::NextToken() {
		SkipHorizontalWhitespaceAndComments();
		const size_t begin = position_;
		const size_t line = line_;
		const size_t column = column_;
		if (position_ >= source_.size()) {
			return {TokenType::EndOfFile, "", 0, line, column, position_};
		}
		if (source_[position_] == '\n') {
			Advance();
			return Make(TokenType::Newline, begin, line, column);
		}
		if (source_[position_] == '#') return ScanDirective();
		if (IsIdentifierStart(static_cast<unsigned char>(source_[position_]))) return ScanIdentifier();
		if (std::isdigit(static_cast<unsigned char>(source_[position_]))) return ScanNumber();

		auto multi = [&](std::string_view spelling, TokenType type) -> std::optional<Token> {
			if (!StartsWith(spelling)) return std::nullopt;
			Advance(spelling.size());
			return Make(type, begin, line, column);
		};
		if (auto token = multi("|||", TokenType::SynchronizedConcurrency)) return *token;
		if (auto token = multi("||", TokenType::InterleavedConcurrency)) return *token;
		if (auto token = multi("<->", TokenType::Equivalence)) return *token;
		if (auto token = multi("&&", TokenType::And)) return *token;
		if (auto token = multi("!=", TokenType::NotEqual)) return *token;
		if (auto token = multi("==", TokenType::Equal)) return *token;
		if (auto token = multi("->", TokenType::Implies)) return *token;
		if (auto token = multi("∧", TokenType::And)) return *token;
		if (auto token = multi("∨", TokenType::Or)) return *token;
		if (auto token = multi("¬", TokenType::Negation)) return *token;
		if (auto token = multi("∀", TokenType::Universal)) return *token;
		if (auto token = multi("∃", TokenType::Existential)) return *token;
		if (auto token = multi("⊃", TokenType::Implies)) return *token;
		if (auto token = multi("≡", TokenType::Equivalence)) return *token;
		if (auto token = multi("≠", TokenType::NotEqual)) return *token;
		if (auto token = multi("π", TokenType::Pi)) return *token;

		const char character = source_[position_];
		Advance();
		switch (character) {
		case ':': return Make(TokenType::Colon, begin, line, column);
		case '.': return Make(TokenType::Dot, begin, line, column);
		case '^': return Make(TokenType::And, begin, line, column);
		case '!': return Make(TokenType::Negation, begin, line, column);
		case '=': return Make(TokenType::Equal, begin, line, column);
		case '(': return Make(TokenType::LParen, begin, line, column);
		case ')': return Make(TokenType::RParen, begin, line, column);
		case ',': return Make(TokenType::Comma, begin, line, column);
		case ';': return Make(TokenType::SemiColon, begin, line, column);
		case '?': return Make(TokenType::QuestionMark, begin, line, column);
		case '*': return Make(TokenType::Star, begin, line, column);
		case '|': return Make(TokenType::NonDet, begin, line, column);
		default: Fail(std::format("unexpected character '{}'", character), line, column);
		}
	}

	std::vector<Token> Lexer::AllTokens() {
		const size_t old_position = position_;
		const size_t old_line = line_;
		const size_t old_column = column_;
		std::vector<Token> tokens;
		for (;;) {
			Token token = NextToken();
			tokens.push_back(token);
			if (token.type == TokenType::EndOfFile) break;
		}
		position_ = old_position;
		line_ = old_line;
		column_ = old_column;
		return tokens;
	}

	[[noreturn]] void Lexer::Fail(std::string_view message, size_t line, size_t column) const {
		throw std::runtime_error(std::format("[ConGologLexer] {} at {}:{}", message, line, column));
	}

	std::ostream& operator<<(std::ostream& os, Lexer& lexer) {
		for (const Token& token : lexer.AllTokens()) {
			os << token << '\n';
		}
		return os;
	}

}
