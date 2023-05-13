#include <gtest/gtest.h>
#include "scs/SituationCalculus/coop_matrix.h"

TEST(ScCoopMatrix, Empty) {
	scs::CoopMatrix cm{5};
	EXPECT_EQ(cm.Lookup(10, 10), false);
	EXPECT_EQ(cm.Lookup(1, 1), false);
	EXPECT_EQ(cm.Lookup(0, 0), false);
}

TEST(ScCoopMatrix, False) {
	scs::CoopMatrix cm{ 5 };
	cm.Add(0, 1);
	EXPECT_EQ(cm.Lookup(10, 10), false);
	EXPECT_EQ(cm.Lookup(1, 2), false);
}

TEST(ScCoopMatrix, True) {
	scs::CoopMatrix cm{ 5 };
	cm.Add(0, 1);
	EXPECT_EQ(cm.Lookup(0, 1), true);
}