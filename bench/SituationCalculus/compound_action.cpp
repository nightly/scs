#include <benchmark/benchmark.h>
#include "scs/SituationCalculus/compound_action.h"

using namespace scs;

static void InvolvesAction(benchmark::State& state) {
	CompoundAction ca({ Action{"Nop"}, Action{"In"}, Action{"Nop"}, Action{"Out"}, Action{"Drill"} });
	for (auto _ : state) {
		bool result = false;
		if (ca.ContainsActionName("In") || ca.ContainsActionName("Out")) {
			result = true;
		}
		benchmark::DoNotOptimize(result);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(InvolvesAction);

static void Names(benchmark::State& state) {
	CompoundAction ca({ Action{"Nop"}, Action{"In"}, Action{"Nop"}, Action{"Out"}, Action{"Drill"} });


	for (auto _ : state) {
		auto result = ca.NamedActions();
		benchmark::DoNotOptimize(result);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(Names);
