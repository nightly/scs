#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "Hinge/Quick/recipe.h"

using namespace scs;
using namespace scs::examples;

class HingeControllerQuick : public benchmark::Fixture {
protected:
	std::unique_ptr<ITopology> topology;
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;
protected:
	void SetUp(const ::benchmark::State& state) {
		//auto resource1 = HingeResource1();
		//graphs.emplace_back(resource1.program, ProgramType::Resource);

		auto resource2 = HingeResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);

		//auto resource3 = HingeResource3();
		//graphs.emplace_back(resource3.program, ProgramType::Resource);

		auto resource4 = HingeResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);

		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();
		topology = std::make_unique<CompleteTopology>(&graphs, true);

		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(1, 2);
		rm.Add(2, 4);

		std::vector<scs::BasicActionTheory> bats{common_bat, resource2.bat, resource4.bat};
		global = CombineBATs(bats, cm, rm);

		auto recipe_prog = HingeRecipeQuick();
		graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	}

	void TearDown(const ::benchmark::State& state) {

	}

};

BENCHMARK_DEFINE_F(HingeControllerQuick, AStar)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 10, .global_cost_limit = 200,
		.stage_transition_limit = 3, .stage_cost_limit = 50, .fairness_limit = 20 };
	Best best(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = best.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
// BENCHMARK_REGISTER_F(HingeControllerQuick, AStar)->Unit(benchmark::kMillisecond)->Iterations(200);
// 3 ms

BENCHMARK_DEFINE_F(HingeControllerQuick, GBFS)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 10, .global_cost_limit = 200,
		.stage_transition_limit = 3, .stage_cost_limit = 50, .fairness_limit = 20 };
	GBFS gbfs(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = gbfs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerQuick, GBFS)->Unit(benchmark::kMillisecond)->Iterations(200);
// 1.02 ms