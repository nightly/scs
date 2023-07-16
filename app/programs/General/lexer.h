#pragma once

#include "scs/ConGolog/Parser/lexer.h"

#include <string>
#include <vector>
#include <iostream>

inline void lexer_prog() {
	std::string program("     #Program   ∀  \0");
	scs::Lexer lex(program);
	std::cout << lex;
}