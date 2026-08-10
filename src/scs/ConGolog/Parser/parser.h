#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "scs/ConGolog/Parser/lexer.h"
#include "scs/ConGolog/resource.h"

namespace scs {

	struct Parser {
	public:
		Lexer lexer;

		explicit Parser(Lexer&& lexer);
		const Token& CurrentToken() const;
		const Token& PeekToken() const;
		Resource Parse();
		void Reset();

		void set_sym_relational_fluents(const std::unordered_set<std::string>& fluents);
		void set_sym_actions(const std::unordered_set<std::string>& actions);
		const std::unordered_set<std::string>& sym_relational_fluents() const;
		const std::unordered_set<std::string>& sym_actions() const;
	private:
		std::vector<Token> tokens_;
		size_t position_ = 0;
		std::unordered_set<std::string> symbols_relational_fluents_;
		std::unordered_set<std::string> symbols_actions_;

		void NextToken();
		bool CheckToken(TokenType type) const;
		bool CheckPeek(TokenType type) const;
		void Match(TokenType type) const;
		void nl();
		[[noreturn]] void Abort(const std::string& message) const;

		friend class FolParser;
	};

}
