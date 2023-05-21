#include <gtest/gtest.h>
#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/successor_extractor.h"

TEST(ScSuccessor, ExtractorSimple) {
	scs::Action a1{"a1"};
	scs::Action a2{"a2"};
	scs::Action a3{"a3"};
	scs::Formula f1 = scs::BinaryConnective(a1, scs::BinaryConnective(a2, a3, scs::BinaryKind::Conjunction), scs::BinaryKind::Disjunction);
	scs::Formula f2 = scs::BinaryConnective(true, f1, scs::BinaryKind::Disjunction);

	scs::SuccessorActionExtractor extractor{};
	std::visit(extractor, f2);
	auto got = extractor.Actions();
	std::unordered_set<std::string> expected{"a1", "a2", "a3"};
	ASSERT_EQ(got, expected);
}