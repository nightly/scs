#include <gtest/gtest.h>
#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/successor_extractor.h"
#include "scs/SituationCalculus/Parser/parser.h"

TEST(ScSuccessor, ExtractorSimple) {
	scs::Formula f2 = scs::ParseScFormula("true or (act(a1()) or (act(a2()) and act(a3())))");

	scs::SuccessorActionExtractor extractor{};
	std::visit(extractor, f2);
	auto got = extractor.Actions();
	ankerl::unordered_dense::set<std::string> expected{"a1", "a2", "a3"};
	ASSERT_EQ(got, expected);
}
