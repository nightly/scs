#include "scs/ConGolog/Parser/parser.h"
#include "scs/ConGolog/Parser/fol.h"

#include <string_view>
#include <string>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Parser/token.h"

namespace scs {

	/**
	 * @brief: Handles a string formula
	 */
	FolParser::FolParser(std::string_view view, TokenType end_token)
		: end_token_(end_token) {
		util_str_parser_ = std::make_unique<Parser>(view);
		parser_ = util_str_parser_.get();
	}

	FolParser::FolParser(Parser& parser, TokenType end_token) 
		: parser_(&parser), end_token_(end_token) {}

	FolParser::~FolParser() {

	}


	/**
	 * @brief: Updates the parser instance by creating a new parser for the new string
	 */
	void FolParser::UpdateInstance(std::string_view view, TokenType end_token) {
		util_str_parser_ = std::make_unique<Parser>(view);
		parser_ = util_str_parser_.get();
		end_token_ = end_token;
	}

	Formula FolParser::ParseFormula() {
		Formula ret;
		size_t paren_level = 1;

		while (!parser_->CheckToken(TokenType::Newline) && !parser_->CheckToken(TokenType::EndOfFile)) {
			
			// Primary
			if (parser_->CheckToken(TokenType::True)) {
				ret = true;
				parser_->NextToken();
			} else if (parser_->CheckToken(TokenType::False)) {
				ret = false;
				parser_->NextToken();
			}

		
			// Custom end check
			if (end_token_ == TokenType::RParen && parser_->CheckPeek(TokenType::RParen) && paren_level == 1) {
				break;
			}
		}
		
		if (parser_->CheckPeek(TokenType::Newline)) {
			parser_->nl();
		}

		return ret;
	}


}