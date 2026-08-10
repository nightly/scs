#pragma once

#include <string>
#include <string_view>

#include "scs/ConGolog/Parser/token.h"
#include "scs/FirstOrderLogic/fol.h"

namespace scs {

	class Parser;

	class FolParser {
	public:
		explicit FolParser(std::string_view view, TokenType end_token = TokenType::Newline);
		FolParser(Parser& parser, TokenType end_token = TokenType::Newline);
		void UpdateInstance(std::string_view view, TokenType end_token = TokenType::Newline);
		Formula ParseFormula();
	private:
		Parser* parser_ = nullptr;
		std::string source_;
		TokenType end_token_ = TokenType::Newline;
	};

}
