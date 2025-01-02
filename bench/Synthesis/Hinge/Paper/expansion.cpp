#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "scs/Synthesis/topology/complete/complete.h"
#include "scs/Synthesis/Actions/cache.h"
#include "scs/Common/log.h"
#include "memory_manager.h"

using namespace scs;
using namespace scs::examples;

class HingeG1TopologyFixture : public benchmark::Fixture {
protected:
	scs::Resource resource1, resource2, resource3, resource4;
	std::vector<CharacteristicGraph> graphs;
	ankerl::unordered_dense::set<Object> objects{ "o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", 
	"o9", "o10", "o11", "o12"};

protected:
	void SetUp(const ::benchmark::State& state) {
		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		resource4 = HingeResource4();
	}
};

void PrintStats(const CompleteTopology& top, const Cache& cache) {
	SCS_STATS("Number of topology states = {}", top.lts().NumOfStates());
	SCS_STATS("Number of topology transitions = {}", top.lts().NumOfTransitions());
	SCS_STATS("Grounded compound actions size = {}", cache.SizeComplete());
}

BENCHMARK_DEFINE_F(HingeG1TopologyFixture, One)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	graphs.emplace_back(resource1.program, ProgramType::Resource);

	for (auto _ : state) {
		CompleteTopology topology(&graphs, true);
		Cache ca_cache = PreExpand(topology, objects);
		
		PrintStats(topology, ca_cache);
		benchmark::DoNotOptimize(ca_cache);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeG1TopologyFixture, Two)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	graphs.emplace_back(resource1.program, ProgramType::Resource);
	graphs.emplace_back(resource2.program, ProgramType::Resource);

	for (auto _ : state) {
		CompleteTopology topology(&graphs, true);
		Cache ca_cache = PreExpand(topology, objects);

		// PrintStats(topology, ca_cache);
		benchmark::DoNotOptimize(ca_cache);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeG1TopologyFixture, Three)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	graphs.emplace_back(resource1.program, ProgramType::Resource);
	graphs.emplace_back(resource2.program, ProgramType::Resource);
	graphs.emplace_back(resource3.program, ProgramType::Resource);

	for (auto _ : state) {
		CompleteTopology topology(&graphs, true);
		Cache ca_cache = PreExpand(topology, objects);

		// PrintStats(topology, ca_cache);
		benchmark::DoNotOptimize(ca_cache);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeG1TopologyFixture, Four)(benchmark::State& state) {
	// Dangerous memory usage (50gb+)
	std::vector<CharacteristicGraph> graphs;
	graphs.emplace_back(resource1.program, ProgramType::Resource);
	graphs.emplace_back(resource2.program, ProgramType::Resource);
	graphs.emplace_back(resource3.program, ProgramType::Resource);
	graphs.emplace_back(resource4.program, ProgramType::Resource);

	for (auto _ : state) {
		CompleteTopology topology(&graphs, true);
		Cache ca_cache = PreExpand(topology, objects);

		// PrintStats(topology, ca_cache);
		benchmark::DoNotOptimize(ca_cache);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_REGISTER_F(HingeG1TopologyFixture, One)->Unit(benchmark::kMillisecond); // note: ms
// BENCHMARK_REGISTER_F(HingeG1TopologyFixture, Two)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeG1TopologyFixture, Three)->Unit(benchmark::kSecond);


// ********************************************** @Notes ***********************************************************************
// We could also consider the difference between abstract topology, and the grounded cache,
// but this is not that significant
// *****************************************************************************************************************************

//BENCHMARK_MAIN();
int main(int argc, char** argv)
{
	::benchmark::RegisterMemoryManager(mm.get());
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::RegisterMemoryManager(nullptr);
}