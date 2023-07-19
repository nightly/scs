#include <gtest/gtest.h>
#include "scs/Synthesis/all_actions.h"

using namespace scs;


TEST(SynthActions, InstantiationsExpand) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", {scs::Object{"c1"}, scs::Variable{"v1"}, scs::Variable{"v2"}}};
	auto x = instantiations.Get(act);

	for (const auto& el : x) {
		std::cout << el << std::endl;
	}
}

TEST(SynthActions, InstantiationsExpandConstant) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", { scs::Object{"c1"}}};
	auto x = instantiations.Get(act);

	for (const auto& el : x) {
		// std::cout << el << std::endl;
	}
}