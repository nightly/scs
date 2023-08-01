#include <benchmark/benchmark.h>
#include "scs/Combinatorics/Actions/instantiations.h"

using namespace scs;

static void Instance0(benchmark::State& state) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"} }};

	for (auto _ : state) {
		auto vec = inst.Get(a1);
		benchmark::DoNotOptimize(vec);
		benchmark::ClobberMemory();
	}
}
// BENCHMARK(Instance0)->Unit(benchmark::kMillisecond);
BENCHMARK(Instance0);
// 230 ns, 168 ns CPU

static void Instance1(benchmark::State& state) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"} }};

	for (auto _ : state) {
		auto vec = inst.Get(a1);
		benchmark::DoNotOptimize(vec);
		benchmark::ClobberMemory();
	}
}
BENCHMARK(Instance1)->Unit(benchmark::kMillisecond);
// 0.046 ms, 0.030 ms CPU

static void Instance2(benchmark::State& state) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"}, Object{"o6"}, Variable{"v3"}, Object{"o4"}, Variable{"v5"} }};

	for (auto _ : state) {
		auto vec = inst.Get(a1);
		benchmark::DoNotOptimize(vec);
		benchmark::ClobberMemory();
	}
}
BENCHMARK(Instance2)->Unit(benchmark::kMillisecond);
// 0.026 ms, 0.019 ms CPU

static void Instance3(benchmark::State& state) {
	ankerl::unordered_dense::set<Object> objects{"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10"};
	scs::ActionInstantiations inst(objects);

	scs::Action a1{"a1", { Variable{"v1"}, Variable{"v2"}, Variable{"v3"}, Variable{"v4"}, Variable{"v5"} }};

	for (auto _ : state) {
		auto vec = inst.Get(a1);
		benchmark::DoNotOptimize(vec);
		benchmark::ClobberMemory();
	}

}
BENCHMARK(Instance3)->Unit(benchmark::kMillisecond);
// 4.5 ms, 3.5 ms CPU

