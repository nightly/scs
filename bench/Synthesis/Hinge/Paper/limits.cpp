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

BENCHMARK_DEFINE_F(HingeControllerLimits, IStageCost)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t arg = state.range(0);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50,
			.stage_cost_limit = arg, .fairness_limit = 20};
		
		GS gs(graphs, graph_recipe, global, *topology, lim);
		auto controller = gs.Synthethise();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IGlobalCost)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t arg = state.range(0);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = arg,
			.stage_transition_limit = 50, .stage_cost_limit = 500, 
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim);
		auto controller = gs.Synthethise();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IStageTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t arg = state.range(0);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim);
		auto controller = gs.Synthethise();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, IGlobalTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
			.stage_transition_limit = 50, .stage_cost_limit = 500,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim);
		auto controller = gs.Synthethise();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, PairCosts)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t global_arg = state.range(0);
		int32_t phase_arg = state.range(1);
		Limits lim{ .global_transition_limit = 2048, .global_cost_limit = global_arg,
			.stage_transition_limit = 50, .stage_cost_limit = phase_arg,
			.fairness_limit = 20 };

		GS gs(graphs, graph_recipe, global, *topology, lim);
		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeControllerLimits, PairTransitions)(benchmark::State& state) {
	for (auto _ : state) {
		int32_t global_arg = state.range(0);
		int32_t phase_arg = state.range(1);
		Limits lim{ .global_transition_limit = global_arg, .global_cost_limit = 8192,
			.stage_transition_limit = phase_arg, .stage_cost_limit = 500,
			.fairness_limit = 20 };
		GS gs(graphs, graph_recipe, global, *topology, lim);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

static size_t num_iterations = 5;
#define GROUP 5

#if GROUP == 1
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(25)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(100)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(150)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(200)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(250)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(300)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(350)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(400)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(500)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(550)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(600)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(650)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(700)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(750)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(800)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(850)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(900)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(950)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageCost)->Arg(1000)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 


#if GROUP == 2
	BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalCost)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalCost)->Arg(100)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 

#if GROUP == 3
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageTransitions)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IStageTransitions)->Arg(100)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 

#if GROUP == 4
	BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalTransitions)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, IGlobalTransitions)->Arg(100)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif 

#if GROUP == 5 
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->Arg(500)->Arg(10)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->Arg(500)->Arg(20)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->Arg(500)->Arg(30)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->Arg(500)->Arg(40)->Unit(benchmark::kSecond)->Iterations(num_iterations);
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairCosts)->Arg(500)->Arg(50)->Unit(benchmark::kSecond)->Iterations(num_iterations);
#endif

#if GROUP == 6
	BENCHMARK_REGISTER_F(HingeControllerLimits, PairTransitions)->Arg(10)->Arg(10)->Unit(benchmark::kSecond)->Iterations(3);
#endif

// ********************************************** @Notes ***********************************************************************
// Do not use DenseRange it causes big memory issue each range for some reason that is not explainable,
// Use repeated BENCHMARK_REGISTER_F with the arguments you want manually listed (either 1 or 2 args if pair)
// *****************************************************************************************************************************

