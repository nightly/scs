#include <gtest/gtest.h>
#include "scs/Common/strings.h"

#include <string>

TEST(Strings, ToLower) {
	std::string str("xYz");
	std::string lower = scs::ToLower(str);
	ASSERT_EQ(str, "xYz");
	ASSERT_EQ(lower, "xyz");
}