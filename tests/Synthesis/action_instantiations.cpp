#include <gtest/gtest.h>
#include "scs/Synthesis/all_actions.h"

using namespace scs;


TEST(SynthActions, InstantiationsExpand) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

}