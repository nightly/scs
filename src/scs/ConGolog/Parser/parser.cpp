#include "scs/ConGolog/Parser/parser.h"

#include <string>
#include <unordered_set>
#include <iostream>

#include "magic_enum.hpp"

#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/Parser/fol.h"

namespace scs {

	// ====================================================================
	// Constructors & generic Parsing functions
	// ====================================================================


	Parser::Parser(Lexer&& lexer)
		: lexer(std::move(lexer)) { 
		this->Reset();
	}

	void Parser::Reset() {
		lexer.Reset();
		// Call NextToken() twice in order to set starting current and peek token
		NextToken();
		NextToken();
	}

	const Token& Parser::CurrentToken() const {
		return current_token_;
	}

	const Token& Parser::PeekToken() const {
		return peek_token_;
	}

	void Parser::NextToken() {
		current_token_ = peek_token_;
		peek_token_ = lexer.NextToken();
	}

	bool Parser::CheckToken(scs::TokenType type) {
		return type == current_token_.type;
	}

	bool Parser::CheckPeek(scs::TokenType type) {
		return type == peek_token_.type;
	}

	// Matches to the given token, abort error if not
	void Parser::Match(scs::TokenType type) {
		if (!CheckToken(type)) {
			Abort("[Parser] Match expected" + std::string(magic_enum::enum_name(type)) + "got" +
				std::string(magic_enum::enum_name(current_token_.type)));
		}
	}

	void Parser::nl() {
		Match(TokenType::Newline);
		while (CheckToken(TokenType::Newline)) {
			NextToken();
		}
	}

	void Parser::set_sym_relational_fluents(const std::unordered_set<std::string>& rfs) {
		symbols_relational_fluents = rfs;
	}

	void Parser::set_sym_actions(const std::unordered_set<std::string>& acts) {
		symbols_actions = acts;
	}

	const std::unordered_set<std::string>& Parser::sym_relational_fluents() const {
		return symbols_relational_fluents;
	}

	const std::unordered_set<std::string>& Parser::sym_actions() const {
		return symbols_actions;
	}

	// ====================================================================
	// ConGolog Parsing
	// ====================================================================


	Resource Parser::Parse() {		
		while (CheckToken(scs::TokenType::Newline)) {
			NextToken();
		}

		while (!CheckPeek(scs::TokenType::EndOfFile)) {
			if (CurrentToken().type == TokenType::DirectiveBAT) {
				SCS_INFO("Parsing BAT");
				ParseDirectiveBAT();
			} else if (CurrentToken().type == TokenType::DirectiveProgram) {
				SCS_INFO("Parsing program");
				ParseDirectiveProgram();
			}
		}

		return Resource();
	}


	void Parser::ParseDirectiveBAT() {
		SCS_CRITICAL("Current token {}", CurrentToken().ToStr());

		NextToken();
	}
	
	void Parser::ParseDirectiveProgram() {
		SCS_CRITICAL("Current token {}", CurrentToken().ToStr());

		NextToken();
	}


	void Parser::Abort(const std::string& message) {
		std::cerr << message << std::endl;
		std::exit(121);
	}
}