#include <gtest/gtest.h>
#include "scs/Synthesis/Actions/cache.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/poss.h"

using namespace scs;

TEST(SynthCompoundActionCache, PureNop) {
	scs::Action Nop{"Nop", {}};
	scs::CompoundAction ca({ Nop, Nop, Nop });
	ankerl::unordered_dense::set<scs::Object> objects{"o1", "o2"};

	scs::Cache cache(objects);
	auto ret = cache.Get(ca);
	EXPECT_EQ(ret.size(), 1);
}

TEST(SynthCompoundActionCache, Example1) {
	scs::Action Nop{"Nop", {}};
	scs::Action In{"In", {Variable{"part"}, Variable{"i"}}};
	scs::Action Out{"Out", {Variable{"part"}, Variable{"i"}}};
	scs::Action Load{"Load", {Variable{"part"}}};

	scs::CompoundAction ca({ Nop, In, Out, Load });
	ankerl::unordered_dense::set<scs::Object> objects{"o1", "o2", "o3", "o4"};

	scs::Cache cache(objects);
	auto ret = cache.Get(ca);
	EXPECT_EQ(ret.size(), 576);
}

TEST(SynthSituationCache, EquivalentFluentStatesShareEntries) {
	BasicActionTheory bat;
	bat.pre["run"] = Poss(std::vector<Term>{}, Formula{true});
	CompoundAction action(Action{"run"});

	RelationalFluent ready;
	ready.AddValuation(true);
	RelationalFluent occupied;
	occupied.AddValuation(false);

	Situation first;
	first.AddFluent("ready", ready);
	first.AddFluent("occupied", occupied);

	Situation equivalent;
	equivalent.AddFluent("occupied", occupied);
	equivalent.AddFluent("ready", ready);
	equivalent.history.emplace_back(Action{"irrelevant_history"});

	Cache cache(bat.objects);
	EXPECT_TRUE(cache.Possible(first, action, bat, true));
	EXPECT_TRUE(cache.Possible(equivalent, action, bat, true));
	EXPECT_EQ(cache.SizeSituationStates(), 1);

	const Situation first_next = cache.Progress(first, action, bat, true);
	const Situation equivalent_next = cache.Progress(equivalent, action, bat, true);
	EXPECT_TRUE(first_next.history.empty());
	EXPECT_TRUE(equivalent_next.history.empty());
	EXPECT_EQ(first_next.Fluents(), equivalent_next.Fluents());
	EXPECT_EQ(cache.SizeSituationStates(), 1);
	EXPECT_EQ(cache.SituationCacheHits(), 2);
}

TEST(SynthSituationCache, DisabledModePreservesHistoryAndBypassesCache) {
	BasicActionTheory bat;
	bat.pre["run"] = Poss(std::vector<Term>{}, Formula{true});
	CompoundAction action(Action{"run"});
	Situation situation;

	Cache cache(bat.objects);
	EXPECT_TRUE(cache.Possible(situation, action, bat, false));
	const Situation next = cache.Progress(situation, action, bat, false);

	EXPECT_EQ(next.Length(), 1);
	EXPECT_EQ(cache.SizeSituationStates(), 0);
	EXPECT_EQ(cache.SituationCacheHits(), 0);
}
