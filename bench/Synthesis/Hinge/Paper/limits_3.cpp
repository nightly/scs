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

	bool shuffling_ = true;
	std::mt19937 rng_{ 2010 };
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
		topology = std::make_unique<CompleteTopology>(&graphs);

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

BENCHMARK_DEFINE_F(HingeControllerLimits, StageCost)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t arg = state.range(0);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = arg,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim, shuffling_, rng_);
		auto controller = gs.Synthethise();
	}
}


BENCHMARK_DEFINE_F(HingeControllerLimits, StageTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t arg = state.range(0);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = arg, .stage_cost_limit = 500,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim, shuffling_, rng_);
		auto controller = gs.Synthethise();
	}
}


BENCHMARK_DEFINE_F(HingeControllerLimits, PairPhaseTransitionsCost)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t global_arg = state.range(0);
		int32_t phase_arg = state.range(1);
		Limits lim{ .global_transition_limit = global_arg, .global_cost_limit = 8192,
			.stage_transition_limit = phase_arg, .stage_cost_limit = 500,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim, shuffling_, rng_);
		auto controller = gs.Synthethise();
	}
}

constexpr size_t num_iterations = 3;
#define GROUP 2

#if GROUP == 1
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(25)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(75)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(100)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(150)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(200)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(250)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(300)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(350)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageCost)->Arg(400)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 

#if GROUP == 2
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(3)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(4)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(5)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(6)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(7)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(8)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(9)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(10)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(20)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, StageTransitions)->Arg(30)->Unit(benchmark::kSecond)->Iterations(num_iterations);

#endif 

#if GROUP == 3
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairPhaseTransitionsCost)->Args({10, 50})->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 


// ********************************************** @Notes ***********************************************************************
// Do not use DenseRange it causes big memory issue each range for some reason that is not explainable,
// Use repeated BENCHMARK_REGISTER_F with the arguments you want manually listed (either 1 or 2 args if pair)
// *****************************************************************************************************************************

