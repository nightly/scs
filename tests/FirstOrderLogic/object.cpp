#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/Common/windows.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

TEST(FolObject, CreateWithDomain) {
	scs::Situation s0;

	scs::Object{ "obj", s0 };
	auto list = s0.objects;

	scs::Object{ "obj", s0 }; 

	auto list2 = s0.objects;
	EXPECT_EQ(list, list2);
}