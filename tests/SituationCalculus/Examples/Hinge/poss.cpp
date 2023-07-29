#include <gtest/gtest.h>

#include <vector>

#include "Hinge/common.h"
#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"
#include "scs/Synthesis/synthesis.h"
#include "scs/SituationCalculus/situation_calculus.h"

using namespace scs::examples;
using namespace scs;

class HingePossTest : public ::testing::Test {
protected:
	HingeCommon common;
	scs::Resource resource1, resource2, resource3, resource4;
	scs::BasicActionTheory global, common_bat;
	scs::CoopMatrix cm{4};
	scs::RoutesMatrix rm{4};

	void SetUp() override {
		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		resource4 = HingeResource4();
		common_bat = HingeCommonBAT();

		cm.Add(0, 1);
		rm.Add(0, 1);
		rm.Add(0, 2);
		rm.Add(0, 3);
		rm.Add(1, 2);
		rm.Add(1, 3);
		rm.Add(2, 3);

		std::vector<scs::BasicActionTheory> bats{resource1.bat, resource2.bat, resource3.bat, resource4.bat, common_bat};
		global = scs::CombineBATs(bats, cm, rm);
	}



	// void TearDown() override {}
};

TEST_F(HingePossTest, SimpleActions) {
	scs::Action nop{"Nop"};
	EXPECT_EQ(global.Initial().Possible(nop, global), true);

	scs::Action load_tube{"Load", { Object{"tube"}, Object{"2"} }};
	EXPECT_EQ(global.Initial().Possible(load_tube, global), true);

	scs::Action release_tube{"Release", { Object{"tube"}, Object{"1"}}};
	EXPECT_EQ(global.Initial().Possible(release_tube, global), false);
}

TEST_F(HingePossTest, SimpleActions2) {
	Situation s = global.Initial();
	s.relational_fluents_["at"].AddValuation({ Object{"tube"}, Object{"1"} }, true);
	scs::Action clamp_tube{"Clamp", { Object{"tube"}, Object{"0.5"}, Object{"1"}} };
	EXPECT_EQ(s.Possible(clamp_tube, global), true);
	auto s_prime = s.Do(clamp_tube, global);

	scs::Action release_tube{"Release", { Object{"tube"}, Object{"1"} }};
	EXPECT_EQ(s.Possible(release_tube, global), false);
	EXPECT_EQ(s_prime.Possible(release_tube, global), true);
}

TEST_F(HingePossTest, CompoundAction1) {
	scs::Action Nop{"Nop"};
	scs::Action load_tube{"Load", { Object{"tube"}, Object{"2"} }};
	scs::CompoundAction ca({ Nop, load_tube, Nop, Nop });
	EXPECT_EQ(global.Initial().Possible(ca, global), true);
}

TEST_F(HingePossTest, TransferAction) {

}