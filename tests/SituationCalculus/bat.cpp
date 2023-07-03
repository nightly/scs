#include <gtest/gtest.h>
#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/situation.h"

TEST(ScBat, InitialSituation) {
	scs::BasicActionTheory bat;
	scs::Situation s;
	bat.SetInitial(s);
}

TEST(ScBat, InitialSituationWithHistory) {
	scs::BasicActionTheory bat;
	scs::Situation s;
	s.history.push_back(scs::Action{"act"});
	ASSERT_DEATH(bat.SetInitial(s), "Initial situation cannot have a history of actions");
}