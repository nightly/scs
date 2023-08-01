#include <benchmark/benchmark.h>
#include "scs/Synthesis/synthesis.h"
#include "Hinge/hinge.h"
#include "scs/Synthesis/pre_expand.h"

#include "scs/Common/windows.h"

using namespace scs;
using namespace scs::examples;

class ExpansionFixture : public benchmark::Fixture {
protected:
	HingeCommon common;
	scs::Resource resource1, resource2, resource3, resource4;
	scs::BasicActionTheory global, common_bat;
	std::vector<CharacteristicGraph> graphs;
	scs::CoopMatrix cm{10};
	scs::RoutesMatrix rm{10};
	
	void SetUp(const ::benchmark::State& state) {
		SetConsoleEncoding();

		resource1 = HingeResource1();
		resource2 = HingeResource2();
		resource3 = HingeResource3();
		// resource4 = HingeResource4();
		common_bat = HingeCommonBAT();

		graphs.emplace_back(resource1.program, ProgramType::Resource);
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		graphs.emplace_back(resource3.program, ProgramType::Resource);
		// graphs.emplace_back(resource4.program, ProgramType::Resource);

		cm.Add(0, 1);
		rm.Add(2, 3);

		std::vector<scs::BasicActionTheory> bats{resource1.bat, resource2.bat, 
			resource3.bat, common_bat};
		global = scs::CombineBATs(bats, cm, rm);
	}

	void TearDown(const ::benchmark::State& state) {
	}

};

BENCHMARK_DEFINE_F(ExpansionFixture, PreExpand)(benchmark::State& state) {
	CompleteTopology topology(&graphs, true);

	for (auto _ : state) {
		auto ret = PreExpand(topology, global.objects);
		benchmark::DoNotOptimize(ret);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(ExpansionFixture, PreExpand)->Unit(benchmark::kMillisecond);
