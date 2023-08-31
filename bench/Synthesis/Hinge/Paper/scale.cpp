#define SCS_MINIMAL_STATS 1

#include <benchmark/benchmark.h>
#include "Hinge/ExtendedGrounded/resource_1.h"
#include "Hinge/ExtendedGrounded/resource_2.h"
#include "Hinge/ExtendedGrounded/resource_3.h"
#include "Hinge/ExtendedGrounded/resource_4.h"
#include "Hinge/ExtendedGrounded/recipe.h"


#include "scs/Synthesis/synthesis.h"

using namespace scs;
using namespace scs::examples;

class HingeScale : public benchmark::Fixture {
protected:
	std::unique_ptr<ITopology> topology;
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;

	bool shuffling_ = true;
	std::mt19937 rng_{ 2010 };
protected:
	void SetUp(const ::benchmark::State& state) {

		auto resource1 = HingeGroundedResource1();
		graphs.emplace_back(resource1.program, ProgramType::Resource);

		auto resource2 = HingeGroundedResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);

		auto resource3 = HingeGroundedResource3();
		graphs.emplace_back(resource3.program, ProgramType::Resource);

		auto resource4 = HingeGroundedResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);

		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();
		topology = std::make_unique<CompleteTopology>(&graphs, true);

		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		cm.Add(1, 4);
		cm.Add(2, 4);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(1, 2);
		rm.Add(2, 4);
		rm.Add(1, 4);

		std::vector<scs::BasicActionTheory> bats{ common_bat, resource1.bat, resource2.bat, resource3.bat };
		global = CombineBATs(bats, cm, rm);

		// auto recipe_prog = HingeExtendedGroundedRecipe();
		// graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	}

	void TearDown(const ::benchmark::State& state) {

	}

};

BENCHMARK_DEFINE_F(HingeScale, 2R)(benchmark::State& state) {

	for (auto _ : state) {
		// auto controller = best.Synthethise();
		// benchmark::DoNotOptimize(controller);
		// benchmark::ClobberMemory();
	}
}

// BENCHMARK_REGISTER_F(HingeScale, 2R)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, 3R)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, 4R)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, 5R)->Unit(benchmark::kMillisecond);


