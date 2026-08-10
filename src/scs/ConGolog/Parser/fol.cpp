#include "scs/ConGolog/Parser/fol.h"

#include <string_view>

#include "scs/ConGolog/Parser/parser.h"
#include "scs/SituationCalculus/Parser/parser.h"

namespace scs {

	FolParser::FolParser(std::string_view view, TokenType end_token)
		: source_(view), end_token_(end_token) {}

	FolParser::FolParser(Parser& parser, TokenType end_token)
		: parser_(&parser), end_token_(end_token) {}

	void FolParser::UpdateInstance(std::string_view view, TokenType end_token) {
		parser_ = nullptr;
		source_ = view;
		end_token_ = end_token;
	}

	Formula FolParser::ParseFormula() {
		if (parser_ == nullptr) return ParseScFormula(source_);
		const size_t start = parser_->position_;
		size_t depth = 0;
		while (!parser_->CheckToken(TokenType::EndOfFile)) {
			const TokenType type = parser_->CurrentToken().type;
			if (depth == 0 && (type == end_token_ || type == TokenType::Newline)) break;
			if (type == TokenType::LParen) {
				++depth;
			} else if (type == TokenType::RParen) {
				if (depth == 0) break;
				--depth;
			}
			parser_->NextToken();
		}
		const Token& first = parser_->tokens_.at(start);
		const size_t end = parser_->CurrentToken().offset;
		return ParseScFormula(std::string_view(parser_->lexer.Source()).substr(first.offset, end - first.offset));
	}

}
