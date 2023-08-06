#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "Hinge/Full/recipe.h"

using namespace scs;
using namespace scs::examples;

class HingeControllerFull : public benchmark::Fixture {
protected:
	std::unique_ptr<ITopology> topology;
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;
protected:
	void SetUp(const ::benchmark::State& state) {

		auto resource1 = HingeResource1();
		graphs.emplace_back(resource1.program, ProgramType::Resource);

		auto resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);

		auto resource3 = HingeResource3();
		graphs.emplace_back(resource3.program, ProgramType::Resource);

		//auto resource4 = HingeResource4();
		//graphs.emplace_back(resource4.program, ProgramType::Resource);

		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();
		topology = std::make_unique<CompleteTopology>(&graphs, true);

		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(1, 2);

		std::vector<scs::BasicActionTheory> bats{common_bat, resource1.bat, resource2.bat, resource3.bat};
		global = CombineBATs(bats, cm, rm);

		auto recipe_prog = HingeRecipe();
		graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	}

	void TearDown(const ::benchmark::State& state) {

	}

};

BENCHMARK_DEFINE_F(HingeControllerFull, AStar)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 50, .global_cost_limit = 200,
		.stage_transition_limit = 4, .stage_cost_limit = 50, .fairness_limit = 20 };
	Best best(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = best.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerFull, AStar)->Unit(benchmark::kMillisecond);
