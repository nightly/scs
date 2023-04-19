#pragma once

#include "scs/ConGolog/resource.h"
#include "scs/ConGolog/Parser/lexer.h"
#include "scs/ConGolog/Parser/token.h"

#include <string>
#include <unordered_set>

namespace scs {

	struct Parser {
	public:
		Lexer lexer;
	private:
		Token current_token_;
		Token peek_token_;

		std::unordered_set<std::string> symbols_relational_fluents;
		std::unordered_set<std::string> symbols_actions;
	public:
		Parser(Lexer&& lexer);
		const Token& CurrentToken() const;
		const Token& PeekToken() const;

		Resource Parse();
		void Reset();
	public:
		void set_sym_relational_fluents(const std::unordered_set<std::string>& rfs);
		void set_sym_actions(const std::unordered_set<std::string>& acts);
		const std::unordered_set<std::string>& sym_relational_fluents() const;
		const std::unordered_set<std::string>& sym_actions() const;
	private:

		void NextToken();
		bool CheckToken(scs::TokenType type);
		bool CheckPeek(scs::TokenType type);
		void Match(scs::TokenType type);
		void nl();

		void ParseDirectiveBAT();
		void ParseDirectiveProgram();

		void Abort(const std::string& message);
	private:
		friend class FolParser;
	};
}