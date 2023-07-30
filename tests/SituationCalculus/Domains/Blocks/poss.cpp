#include <gtest/gtest.h>
#include "scs/SituationCalculus/poss.h"


class BlocksPossTest : public ::testing::Test {
protected:
	void SetUp() override {
	}

	scs::RelationalFluent clear;

	scs::Action move_b_to_b{ "move_b_to_b", {scs::Variable{"x"}, scs::Variable{"y"}, scs::Variable{"z"}}};
	scs::Action move_b_to_t{ "move_b_to_t", {scs::Variable{"x"}, scs::Variable{"y"}}};
	scs::Action move_t_to_b{ "move_t_to_b", {scs::Variable{"x"}, scs::Variable{"y"}}};

	// void TearDown() override {}
};