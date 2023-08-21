#include <benchmark/benchmark.h>
#include "Hinge/hinge.h"
#include "Hinge/Full/recipe.h"

using namespace scs;
using namespace scs::examples;

class HingeControllerLimits : public benchmark::Fixture {
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

};

/*** Base limits
	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
		.stage_transition_limit = 50, .stage_cost_limit = 500, .fairness_limit = 20 };
***/

BENCHMARK_DEFINE_F(HingeControllerLimits, IStageCost)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50,
			.stage_cost_limit = static_cast<int32_t>(state.range(0)), .fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IGlobalCost)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500, 
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IStageTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IGlobalTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, PairCosts)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, PairTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}


BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->DenseRange(50, 1000, 100)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalCost)->DenseRange(50, 1000, 50)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeControllerLimits, IStageTransitions)->DenseRange(50, 1000, 50)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalTransitions)->DenseRange(50, 1000, 50)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->DenseRange(50, 1000, 50)->Unit(benchmark::kSecond);
// BENCHMARK_REGISTER_F(HingeControllerLimits, PairTransitions)->DenseRange(50, 1000, 50)->Unit(benchmark::kSecond);
