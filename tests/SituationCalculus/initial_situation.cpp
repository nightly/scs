#include <gtest/gtest.h>
#include "scs/SituationCalculus/initial_situation.h"
#include "scs/SituationCalculus/situation.h"

TEST(ScInitialSituation, InitialSituation) {
	scs::Situation s;
	scs::InitialSituation s0(s);
}

TEST(ScInitialSituation, InitialSituationWithHistory) {
	scs::Situation s;
	s.history.push_back({ "act" });
	ASSERT_DEATH(scs::InitialSituation s0(s), "Initial situation cannot have a history of actions");
}