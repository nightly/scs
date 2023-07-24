#include <gtest/gtest.h>
#include "scs/Synthesis/all_actions.h"

using namespace scs;

inline void print_vec(const std::vector<Action>& acts) {
	for (const auto& el : acts) {
		std::cout << el << std::endl;
	}
}

TEST(SynthActions, InstantiationsExpand) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", {scs::Variable{"o1"}, scs::Variable{"v1"}, scs::Variable{"v2"}}};
	auto x = instantiations.Get(act);
}

TEST(SynthActions, InstantiationsExpand2) {
	std::unordered_set<Object> objects{"o1", "c2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", { scs::Object{"c2"}, scs::Variable{"v1"}, scs::Variable{"v2"} }};
	auto x = instantiations.Get(act);
	print_vec(x);
}

TEST(SynthActions, InstantiationsExpandConstant) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", { scs::Object{"c1"}}};
	auto x = instantiations.Get(act);

}

TEST(SynthActions, PermAmounts1) {
	std::unordered_set<Object> objects{"o1", "o2", "o3"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"} }};
	auto vec_a1 = inst.Get(a1);
	ASSERT_EQ(vec_a1.size(), 3);
	ASSERT_EQ(vec_a1, std::vector<Action>({
		Action("a1", {Object{"o1"}}),
		Action("a1", {Object{"o2"}}),
		Action("a1", {Object{"o3"}})
	}));

	scs::Action a2{"a2", { Variable{"v1"}, Variable{"v2"}}};
	auto vec_a2 = inst.Get(a2);
	ASSERT_EQ(vec_a2.size(), 6);

	scs::Action a3{"a3", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"}}};
	auto vec_a3 = inst.Get(a3);
	ASSERT_EQ(vec_a3.size(), 6);

	scs::Action a4{"a4", { Object{"o1"} }};
	auto vec_a4 = inst.Get(a4);
	ASSERT_EQ(vec_a4.size(), 1);
	ASSERT_EQ(vec_a4[0], scs::Action("a4", { Object{"o1"} }));

	//scs::Action a5{"a5", { Object{"o1"}, Variable{"v1"}}};
	//auto vec_a5 = inst.Get(a5);
	//ASSERT_EQ(vec_a5.size(), 3);

	//scs::Action a6{"a6", { Object{"o1"}, Variable{"v1"}, Object{"o3"}}};
	//auto vec_a6 = inst.Get(a6);
	//ASSERT_EQ(vec_a6.size(), 3);
}

TEST(SynthActions, PermAmounts2) {
	std::unordered_set<Object> objects{"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"} }};
	auto vec_a1 = inst.Get(a1);
	ASSERT_EQ(vec_a1.size(), 720);

	scs::Action a2{"a2", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"}, Variable{"v4"}, Variable{"v5"}}};
	auto vec_a2 = inst.Get(a2);
	ASSERT_EQ(vec_a2.size(), 30240);
}