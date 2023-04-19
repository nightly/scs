#pragma once

#include <string>
#include <string_view>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Parser/token.h"

namespace scs {

	class Parser;

	class FolParser {
	private:
		Parser* parser_;

		// temp parser pointer for strings passed in
		std::unique_ptr<Parser> util_str_parser_;

		// End token
		TokenType end_token_ = TokenType::Newline;
	public:
		FolParser(std::string_view view, TokenType end_token = TokenType::Newline);
		FolParser(Parser& parser, TokenType end_token = TokenType::Newline);
		~FolParser();

		void UpdateInstance(std::string_view view, TokenType end_token = TokenType::Newline);

		Formula ParseFormula();
	private:
		void Primary();
		void Unary();
		void Quantification();
	};
}