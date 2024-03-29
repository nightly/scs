#include <gtest/gtest.h>

#include <vector>

#include "Hinge/hinge.h"
#include "scs/SituationCalculus/utils.h"

#include "scs/Common/windows.h"

using namespace scs::examples;
using namespace scs;

class HingeRecipeTest : public ::testing::Test {
protected:
	HingeCommon common;
	scs::BasicActionTheory global;
	scs::CoopMatrix cm{10};
	scs::RoutesMatrix rm{10};
	Action Nop{ "Nop" };

	void SetUp() override {
		SetConsoleEncoding();

		auto resource1 = HingeResource1();
		auto resource2 = HingeResource2();
		auto resource3 = HingeResource3();
		// auto resource4 = HingeResource4();
		auto common_bat = HingeCommonBAT();

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
	std::vector<CompoundAction> targets;

	sit.emplace_back(global.Initial());

	// Load(tube) || Load(brass)
	ca.emplace_back(std::vector<Action>{Nop, Action("Load", {Object{"brass"}, Object{"2"}}), Nop});
	targets.emplace_back(std::vector<Action>{ Action("Load", {Object{"brass"}}) });
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	ASSERT_TRUE(UnifyActions(ca.back(), targets.back()));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{Nop, Action("Load", {Object{"tube"}, Object{"2"}}), Action("AttachBit", {Object{"5mm"}, Object{"3"}})});
	targets.emplace_back(std::vector<Action>{ Action("Load", { Object{"tube"} }) });
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	ASSERT_TRUE(UnifyActions(ca.back(), targets.back()));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	// Clamp(brass) ||| RadialDrill(brass, 5);
	targets.emplace_back(std::vector<Action>{Action{"Clamp", { Object{"brass"} }}, Action{ "RadialDrill", { Object{"brass"}, Object{"5mm"}} }});

	ca.emplace_back(std::vector<Action>{ Action("In", {Object{"brass"}, Object{"1"}}), Action("Out", {Object{"brass"}, Object{"2"}}), Nop});
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{ Action("Clamp", {Object{"brass"}, Object{"5"}, Object{"1"}}), Nop, Action("RadialDrill", { Object{"brass"},
		Object{"5mm"}, Object{"1.5"}, Object{"3"}})});
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	ASSERT_TRUE(UnifyActions(ca.back(), targets.back()));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	//ApplyAdhesive(tube, brass)
	targets.emplace_back(std::vector<Action>{ Action("ApplyAdhesive", { Object{"brass"}, Object{"tube"} }) });

	ca.emplace_back(std::vector<Action>{Action("Release", {Object{"brass"}, Object{"1"}}), Nop, Nop });
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	ca.emplace_back(std::vector<Action>{Nop, Nop, scs::Action("ApplyAdhesive", {Object{"brass"}, Object{"tube"}, Object{"3"}})});
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	ASSERT_TRUE(UnifyActions(ca.back(), targets.back()));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	// Store(brass, ok)
	targets.emplace_back(std::vector<Action>{ Action("Store", { Object{"brass"}, Object{"ok"} })});
	ca.emplace_back(std::vector<Action>{Action("Store", { Object{"brass"}, Object{"ok"}, Object{"1"}}), Nop, Nop});
	ASSERT_TRUE(sit.back().Possible(ca.back(), global));
	ASSERT_TRUE(Legal(ca.back(), targets.back(), global));
	ASSERT_TRUE(UnifyActions(ca.back(), targets.back()));
	sit.emplace_back(sit.back().Do(ca.back(), global));

	// std::cout << global;
	std::cout << sit.back();
}
