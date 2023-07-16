#pragma once

#include <string>
#include <string_view>
#include <iostream>

#include "scs/ConGolog/Parser/utf8.h"

inline void ExpTokensUtf8Prog_() {
	std::string tokens("a∀\n\n\n\n\n");

	std::string x = "tes\n\nst";
	std::string_view view(x.c_str() + 2, 4);
	std::cout << view;
}