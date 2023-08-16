#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "Hinge/Full/recipe.h"

using namespace scs;
using namespace scs::examples;

class HingeControllerFullLimits : public benchmark::Fixture {
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

		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();
		topology = std::make_unique<CompleteTopology>(&graphs, true);

		CoopMatrix cm(10);
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		RoutesMatrix rm(10);
		rm.Add(1, 2);

		std::vector<scs::BasicActionTheory> bats{ common_bat, resource1.bat, resource2.bat, resource3.bat };
		global = CombineBATs(bats, cm, rm);

		auto recipe_prog = HingeRecipe();
		graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	}

	void TearDown(const ::benchmark::State& state) {}
};

/*
	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
		.stage_transition_limit = 25, .stage_cost_limit = 500, .fairness_limit = 20 };
*/

BENCHMARK_DEFINE_F(HingeControllerFullLimits, GsGlobalTransitions)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = static_cast<int32_t>(state.range(0)), .global_cost_limit = 8192,
		.stage_transition_limit = 25, .stage_cost_limit = 500, .fairness_limit = 20 };
	GS gs(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerFullLimits, GsGlobalTransitions)->DenseRange(50, 1000, 50)->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(HingeControllerFullLimits, GsStageTransitions)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
		.stage_transition_limit = static_cast<int32_t>(state.range(0)), .stage_cost_limit = 500, .fairness_limit = 20 };
	GS gs(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerFullLimits, GsStageTransitions)->DenseRange(5, 50, 1)->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(HingeControllerFullLimits, GsGlobalCost)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = static_cast<int32_t>(state.range(0)),
		.stage_transition_limit = 25, .stage_cost_limit = 500, .fairness_limit = 20 };
	GS gs(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerFullLimits, GsGlobalCost)->DenseRange(300, 3000, 100)->Unit(benchmark::kMillisecond);


BENCHMARK_DEFINE_F(HingeControllerFullLimits, GsStageCost)(benchmark::State& state) {
	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
		.stage_transition_limit = 25, .stage_cost_limit = static_cast<int32_t>(state.range(0)), .fairness_limit = 20};
	GS gs(graphs, graph_recipe, global, *topology, lim);

	for (auto _ : state) {
		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}
BENCHMARK_REGISTER_F(HingeControllerFullLimits, GsStageCost)->DenseRange(20, 1000, 20)->Unit(benchmark::kMillisecond);


