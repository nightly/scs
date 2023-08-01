#include <gtest/gtest.h>
#include "scs/Synthesis/Actions/cache.h"

using namespace scs;

TEST(SynthCompoundActionCache, PureNop) {
	scs::Action Nop{"Nop", {}};
	scs::CompoundAction ca({ Nop, Nop, Nop });
	ankerl::unordered_dense::set<scs::Object> objects{"o1", "o2"};

	scs::CompoundActionCache cache(objects);
	auto ret = cache.Get(ca);
	EXPECT_EQ(ret.size(), 1);
}

TEST(SynthCompoundActionCache, Example1) {
	scs::Action Nop{"Nop", {}};
	scs::Action In{"In", {Variable{"part"}, Variable{"i"}}};
	scs::Action Out{"Out", {Variable{"part"}, Variable{"i"}}};
	scs::Action Load{"Load", {Variable{"part"}}};

	scs::CompoundAction ca({ Nop, In, Out, Load });
	ankerl::unordered_dense::set<scs::Object> objects{"o1", "o2", "o3", "o4"};

	scs::CompoundActionCache cache(objects);
	auto ret = cache.Get(ca);
	EXPECT_EQ(ret.size(), 576);
}