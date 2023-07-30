#include <gtest/gtest.h>

#include <vector>

#include "Hinge/hinge.h"
#include "scs/SituationCalculus/utils.h"

using namespace scs::examples;
using namespace scs;

class HingeRecipeTest : public ::testing::Test {
protected:
	HingeCommon common;
	scs::Resource resource1, resource2, resource3, resource4;
	scs::BasicActionTheory global, common_bat;
	scs::CoopMatrix cm{10};
	scs::RoutesMatrix rm{10};
	Action Nop{ "Nop" };

	void SetUp() override {
		SetConsoleEncoding();

		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		// resource4 = HingeResource4();
		common_bat = HingeCommonBAT();

		cm.Add(1, 3);
		cm.Add(2, 3);

		rm.Add(1, 2);

		std::vector<scs::BasicActionTheory> bats{common_bat, resource1.bat, resource2.bat,
			resource3.bat};
		global = scs::CombineBATs(bats, cm, rm);
	}

};

TEST_F(HingeRecipeTest, Follow) {

	std::vector<Situation> sit;
	std::vector<CompoundAction> ca;

	sit.emplace_back(global.Initial());

	// Load(tube) || Load(brass)
	ca.emplace_back(std::vector<Action>{Nop, Action("Load", {Object{"brass"}, Object{"2"}}), Nop});
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{Nop, Action("Load", {Object{"tube"}, Object{"2"}}), Action("AttachBit", {Object{"5mm"}, Object{"3"}})});
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	// Clamp(brass) ||| RadialDrill(brass, 5);
	ca.emplace_back(std::vector<Action>{ Action("In", {Object{"brass"}, Object{"1"}}), Action("Out", {Object{"brass"}, Object{"2"}}), Nop});
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{ Action("Clamp", {Object{"brass"}, Object{"5"}, Object{"1"}}), Nop, Action("RadialDrill", { Object{"brass"},
		Object{"5mm"}, Object{"1.5"}, Object{"3"}})});
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	//ApplyAdhesive(tube, brass)
	ca.emplace_back(std::vector<Action>{Action("Release", {Object{"brass"}, Object{"1"}}), Nop, Nop });
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{Nop, Nop, scs::Action("ApplyAdhesive", {Object{"brass"}, Object{"tube"}, Object{"3"}})});
	ASSERT_EQ(sit.back().Possible(ca.back(), global), true);
	sit.emplace_back(sit.back().Do(ca.back(), global));

	std::cout << global;
	// std::cout << sit.back();
}
