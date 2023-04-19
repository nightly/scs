#include <gtest/gtest.h>
#include "scs/ConGolog/Parser/parser.h"

#include <string>
#include <vector>

#include "scs/Common/io.h"

TEST(CgParserShip, Recipe) {
	const std::string program = R"V0G0N(
	#Program
	load(f, 4, steel, 810, 756, 29) ;
	(load(b, 2, steel, 312, 23, 20) || drill(f, .3, 200, 123, 89, 21));
	apply_glue(b, str_adh); place(b, f, fb, 7, 201, 29);
	if ¬precision(hole(f, 123, 89, 21), high) do
		reaming(fb, 0.3, 123, 89, 21)
	rivert(fb, 213, 89, 21); store(fb, ok)
    )V0G0N";


}