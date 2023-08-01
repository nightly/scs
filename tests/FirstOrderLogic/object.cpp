#include <gtest/gtest.h>
#include <iostream>
#include <format>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/windows.h"

using namespace scs;

TEST(FolObject, CreateWithDomain) {
	scs::Situation s0;
	scs::BasicActionTheory bat;

	scs::Object{ "obj", bat };
	auto list = bat.objects;

	scs::Object{ "obj", bat }; 

	auto list2 = bat.objects;
	EXPECT_EQ(list, list2);
}