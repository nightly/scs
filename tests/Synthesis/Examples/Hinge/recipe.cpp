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

		cm.Add(0, 1);
		rm.Add(1, 2);

		std::vector<scs::BasicActionTheory> bats{common_bat, resource1.bat, resource2.bat,
			resource3.bat};
		global = scs::CombineBATs(bats, cm, rm);
	}

};

TEST_F(HingeRecipeTest, Follow) {

	Situation s_initial = global.Initial();

	CompoundAction ca1({Nop, Action("Load", {Object{"brass"}, Object{"2"}}), Nop});
	ASSERT_EQ(s_initial.Possible(ca1, global), true);
	Situation s1 = s_initial.Do(ca1, global);

	CompoundAction ca2({Nop, Action("Load", {Object{"tube"}, Object{"2"}}), Nop});
	ASSERT_EQ(s1.Possible(ca2, global), true);
	Situation s2 = s1.Do(ca2, global);

	// Clamp(brass) ||| RadialDrill(brass, 5);
	CompoundAction ca3({ Action("In", {Object{"brass"}, Object{"1"}}), Action("Out", {Object{"brass"}, Object{"2"}}), Nop});
	ASSERT_EQ(s2.Possible(ca3, global), true);
	Situation s3 = s2.Do(ca3, global);

	CompoundAction ca4({ Action("Clamp", {Object{"brass"}, Object{"5"}, Object{"1"}}), Nop, Action("RadialDrill", { Object{"brass"}, 
		Object{"5mm"}, Object{"1.5"}, Object{"3"}})});
	ASSERT_EQ(s3.Possible(ca4, global), true);
	Situation s4 = s3.Do(ca4, global);

	//ApplyAdhesive(tube, brass)


	// std::cout << global;
	// std::cout << s2;
}
// 		scs::Action RadialDrill{"RadialDrill", { Variable{"part"},  Variable{"bit"}, Variable{"diameter"}, Object{"3"} }};
