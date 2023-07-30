#include <gtest/gtest.h>

#include <vector>

#include "Hinge/hinge.h"

using namespace scs::examples;
using namespace scs;

class HingeSynTest : public ::testing::Test {
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

};

TEST_F(HingeSynTest, Part1) {

}

TEST_F(HingeSynTest, Part2) {

}

