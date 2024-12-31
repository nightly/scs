#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "scs/Synthesis/topology/complete/complete.h"
#include "memory_manager.h"

using namespace scs;
using namespace scs::examples;

class TopologyFixture : public benchmark::Fixture {
protected:
	scs::Resource resource1, resource2, resource3, resource4;
	std::vector<CharacteristicGraph> graphs;

protected:
	void SetUp(const ::benchmark::State& state) {

		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		resource4 = HingeResource4();

		graphs.emplace_back(resource1.program, ProgramType::Resource);
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		graphs.emplace_back(resource3.program, ProgramType::Resource);
		graphs.emplace_back(resource4.program, ProgramType::Resource);

	}
	
	void TearDown(const ::benchmark::State& state) {
	}

};

BENCHMARK_DEFINE_F(TopologyFixture, CompleteRecursive)(benchmark::State& state) {
	for (auto _ : state) {
		CompleteTopology topology(&graphs, true);
		benchmark::DoNotOptimize(topology);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(TopologyFixture, CompleteRecursive)->Unit(benchmark::kMillisecond);


BENCHMARK_DEFINE_F(TopologyFixture, CompleteIterative)(benchmark::State& state) {
	for (auto _ : state) {
		CompleteTopology topology(&graphs, false);
		benchmark::DoNotOptimize(topology);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(TopologyFixture, CompleteIterative)->Unit(benchmark::kMillisecond);

//BENCHMARK_MAIN();
int main(int argc, char** argv)
{
	::benchmark::RegisterMemoryManager(mm.get());
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::RegisterMemoryManager(nullptr);
}