#include <gtest/gtest.h>
#include "scs/Common/io.h"

TEST(IO, ReadUnicodeFile) {
	std::string got = scs::ReadIntoString("../../tests/Common/Data/unicode.txt");
	std::string expected = "∧\0";
	ASSERT_EQ(got, expected);
}

TEST(IO, DISABLED_ReadAll) {
	scs::ReadAll("../../data/E1", ".cgl");
}