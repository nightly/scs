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

TEST(FolObject, RigidConstantsAndIdentifiersAreDisjoint) {
	const Object rigid = Object::Rigid("same-spelling");
	const Object identifier = Object::Identifier("same-spelling");
	EXPECT_NE(rigid, identifier);
	ObjectSet objects{rigid, identifier};
	EXPECT_EQ(objects.size(), 2);
}
