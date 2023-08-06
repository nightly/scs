#include <gtest/gtest.h>
#include "scs/Combinatorics/Actions/instantiations.h"
#include "scs/Common/print.h"

using namespace scs;

template <typename T>
void print_vec(const std::vector<T>& vec) {
	for (const auto& el : vec) {
		std::cout << el << std::endl;
	}
}

TEST(ComActions, InstantiationsExpand) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", {scs::Variable{"o1"}, scs::Variable{"v1"}, scs::Variable{"v2"}}};
	auto x = instantiations.Get(act);
}

TEST(ComActions, InstantiationsExpand2) {
	ankerl::unordered_dense::set<Object> objects{"o1", "c2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", { scs::Object{"c2"}, scs::Variable{"v1"}, scs::Variable{"v2"} }};
	auto x = instantiations.Get(act);
	// print_vec(x);
}

TEST(ComActions, FetchPermutation) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	Permutation p{ .r = 2 };
	auto x = instantiations.FetchPermutation(p);
	std::cout << x;
}

TEST(ComActions, InstantiationsExpandConstant) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4"};
	scs::ActionInstantiations instantiations(objects);

	scs::Action act{"a1", { scs::Object{"c1"}}};
	auto x = instantiations.Get(act);
}

TEST(ComActions, PermAmounts1) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"} }};
	ASSERT_EQ(inst.Cardinality(a1), 3);
	auto vec_a1 = inst.Get(a1);
	ASSERT_EQ(vec_a1.size(), 3);
	ASSERT_EQ(vec_a1, std::vector<Action>({
		Action("a1", {Object{"o1"}}),
		Action("a1", {Object{"o2"}}),
		Action("a1", {Object{"o3"}})
	}));

	scs::Action a2{"a2", { Variable{"v1"}, Variable{"v2"}}};
	ASSERT_EQ(inst.Cardinality(a2), 6);

	scs::Action a3{"a3", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"}}};
	ASSERT_EQ(inst.Cardinality(a3), 6);

	scs::Action a4{"a4", { Object{"o1"} }};
	auto vec_a4 = inst.Get(a4);
	ASSERT_EQ(vec_a4.size(), 1);
	ASSERT_EQ(inst.Cardinality(a4), 1);
	ASSERT_EQ(vec_a4[0], scs::Action("a4", { Object{"o1"} }));

	scs::Action a5{"a5", { Object{"o1"}, Variable{"v1"}}};
	ASSERT_EQ(inst.Cardinality(a5), 2);

	scs::Action a6{"a6", { Object{"o1"}, Variable{"v1"}, Object{"o3"}}};
	ASSERT_EQ(inst.Cardinality(a6), 1);

}

TEST(ComActions, PermAmounts2) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"} }};
	ASSERT_EQ(inst.Cardinality(a1), 720);

	scs::Action a2{"a2", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"}, Variable{"v4"}, Variable{"v5"}}};
	ASSERT_EQ(inst.Cardinality(a2), 30240);

	// r = 3, n = (10 - 2) = 8
	// 8! / (8-3)!
	scs::Action a3{"a3", { Variable{"v1"}, Object{"o6"}, Variable{"v3"}, Object{"o4"}, Variable{"v5"} }};
	ASSERT_EQ(inst.Cardinality(a3), 336);
}