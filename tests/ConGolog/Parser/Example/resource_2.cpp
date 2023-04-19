#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/parser.h"

#include <string>
#include <vector>

#include "scs/Common/io.h"

TEST(CgParserExample, Resource2) {
	const std::string program = R"V0G0N(
	#BAT
	Poss(in(part,i), s) ≡ part(part, s) ∧ withinReach(part,i, s) ∧ ¬∃p. at(p,i, s)
	Poss(out(part,i), s) ≡ part(part, s) ∧ at(part,i, s)

	#Program
	loop:
	nop | (in(part,2) ; (nop | HoldInPlace(part, force,2))* ; out(part,2))
    )V0G0N";

	 scs::Lexer lex(program);
	 std::cout << lex;

}

// 	nop | (in(part,2) ; (nop | HoldInPlace(part, force,2))∗ ; out(part,2))
