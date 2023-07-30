#include <gtest/gtest.h>

#include <vector>

#include "Hinge/hinge.h"

using namespace scs::examples;
using namespace scs;

class HingePossTest : public ::testing::Test {
protected:
	HingeCommon common;
	scs::Resource resource1, resource2, resource3, resource4;
	scs::BasicActionTheory global, common_bat;
	scs::CoopMatrix cm{10};
	scs::RoutesMatrix rm{10};

	void SetUp() override {
		SetConsoleEncoding();

		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		resource4 = HingeResource4();
		common_bat = HingeCommonBAT();

		cm.Add(0, 1);

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
	scs::Action clamp_tube{"Clamp", { Object{"tube"}, Object{"5"}, Object{"1"}} };
	EXPECT_EQ(s.Possible(clamp_tube, global), true);
	auto s_prime = s.Do(clamp_tube, global);

	std::cout << s_prime;

	scs::Action release_tube{"Release", { Object{"tube"}, Object{"1"} }};
	EXPECT_EQ(s_prime.Possible(release_tube, global), true);
}

TEST_F(HingePossTest, CompoundAction1) {
	scs::Action Nop{"Nop"};
	scs::Action load_tube{"Load", { Object{"tube"}, Object{"2"} }};
	scs::CompoundAction ca({ Nop, load_tube, Nop, Nop });
	EXPECT_EQ(global.Initial().Possible(ca, global), true);
}

TEST_F(HingePossTest, TransferAction) {
	scs::Action Nop{"Nop"};
	scs::Action load{"Load", { Object{"tube"}, Object{"2"} }};
	scs::CompoundAction ca({ Nop, load, Nop, Nop });
	EXPECT_EQ(global.Initial().Possible(ca, global), true);

	// LogModeTracing();
	Situation s = global.Initial().Do(ca, global);
	EXPECT_EQ(s.relational_fluents_["at"].Valuation({ Object{"tube"}, Object{"2"} }), true);

	scs::Action Out{"Out", {Object{ "tube" }, Object{ "2" }}};
	scs::Action In{"In", {Object{ "tube" }, Object{ "3" }}};
	scs::CompoundAction transfer({ Nop, Out, In, Nop });
	Situation s_prime;
	if (s.Possible(transfer, global)) {
		s_prime = s.Do(transfer, global);
	} else {
		s_prime = s;
	}	
	EXPECT_EQ(s_prime.relational_fluents_["at"].Valuation({Object{"tube"}, Object{"2"}}), false);
	EXPECT_EQ(s_prime.relational_fluents_["at"].Valuation({Object{"tube"}, Object{"3"}}), true);
}

TEST_F(HingePossTest, TransferAction2) { // Loads brass tries transfer tube
	scs::Action Nop{"Nop"};
	scs::Action load{"Load", { Object{"brass"}, Object{"2"} }};
	scs::CompoundAction ca({ Nop, load, Nop, Nop });
	EXPECT_EQ(global.Initial().Possible(ca, global), true);

	// LogModeTracing();
	Situation s = global.Initial().Do(ca, global);
	EXPECT_EQ(s.relational_fluents_["at"].Valuation({ Object{"brass"}, Object{"2"} }), true);

	scs::Action Out{"Out", { Object{ "tube" }, Object{ "2" } }};
	scs::Action In{"In", { Object{ "tube" }, Object{ "3" } }};
	scs::CompoundAction transfer({ Nop, Out, In, Nop });
	
	Situation s_prime;
	if (s.Possible(transfer, global)) {
		s_prime = s.Do(transfer, global);
	} else {
		s_prime = s;
	}
	EXPECT_EQ(s_prime.relational_fluents_["at"].Valuation({ Object{"brass"}, Object{"2"} }), true);
	EXPECT_EQ(s_prime.relational_fluents_["at"].Valuation({ Object{"brass"}, Object{"3"} }), false);
}

TEST_F(HingePossTest, Boundedness) {

}
