#define SCS_MINIMAL_STATS 1

#include <benchmark/benchmark.h>
#include "Hinge/ExtendedGrounded/resource_1.h"
#include "Hinge/ExtendedGrounded/resource_2.h"
#include "Hinge/ExtendedGrounded/resource_3.h"
#include "Hinge/ExtendedGrounded/resource_4.h"

#include "Hinge/ExtendedGrounded/recipe.h"
#include "Hinge/Quick/recipe.h"
#include "Hinge/Full/recipe.h"

#include "scs/Synthesis/synthesis.h"

using namespace scs;
using namespace scs::examples;

class HingeScale : public benchmark::Fixture {
protected:
	scs::BasicActionTheory common_bat;
	CoopMatrix cm{10};
	RoutesMatrix rm{10};

	bool shuffling_ = true;
	std::mt19937 rng_{ 2010 };
protected:
	void SetUp(const ::benchmark::State& state) {
		common_bat = HingeCommonBAT();
		
		cm.Add(1, 2);
		cm.Add(1, 3);
		cm.Add(2, 3);
		cm.Add(1, 4);
		cm.Add(2, 4);
		cm.Add(2, 3);

		rm.Add(1, 2);
		rm.Add(2, 4);
		rm.Add(1, 4);
	}

	void TearDown(const ::benchmark::State& state) {

	}

};

BENCHMARK_DEFINE_F(HingeScale, 2R)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	CharacteristicGraph graph_recipe;

	//
	auto resource2 = HingeGroundedResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);
	auto resource4 = HingeGroundedResource4();
	graphs.emplace_back(HingeGroundedResource4Cg());
	auto recipe_prog = HingeRecipeQuick();
	graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	//

	std::vector<scs::BasicActionTheory> bats{ common_bat, resource2.bat, resource4.bat };

	Limits lim{ .global_transition_limit = 10, .global_cost_limit = 200,
		.stage_transition_limit = 3, .stage_cost_limit = 50, .fairness_limit = 20 };

	for (auto _ : state) {
		scs::BasicActionTheory global = CombineBATs(bats, cm, rm);
		CompleteTopology topology(&graphs);
		GS gs(graphs, graph_recipe, global, topology, lim, shuffling_, rng_);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeScale, 3R)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;

	//
	auto resource1 = HingeGroundedResource1();
	graphs.emplace_back(HingeGroundedResource1Cg());

	auto resource2 = HingeGroundedResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);

	auto resource3 = HingeGroundedResource3();
	graphs.emplace_back(resource3.program, ProgramType::Resource);

	auto recipe_prog = HingeRecipe();
	graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	//

	std::vector<scs::BasicActionTheory> bats{ common_bat, resource1.bat, resource2.bat, resource3.bat };

	Limits lim{ .global_transition_limit = 2048, .global_cost_limit = 8192,
		.stage_transition_limit = 50, .stage_cost_limit = 500, .fairness_limit = 20 };

	for (auto _ : state) {
		scs::BasicActionTheory global = CombineBATs(bats, cm, rm);
		CompleteTopology topology(&graphs);
		GS gs(graphs, graph_recipe, global, topology, lim, shuffling_, rng_);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeScale, 4R)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;

	//
	auto resource1 = HingeGroundedResource1();
	graphs.emplace_back(HingeGroundedResource1Cg());

	auto resource2 = HingeGroundedResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);

	auto resource3 = HingeGroundedResource3();
	graphs.emplace_back(resource3.program, ProgramType::Resource);

	auto resource4 = HingeGroundedResource4();
	graphs.emplace_back(HingeGroundedResource4Cg());

	auto recipe_prog = HingeExtendedGroundedRecipe();
	graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	//

	std::vector<scs::BasicActionTheory> bats{ common_bat, resource1.bat, resource2.bat, resource3.bat, resource4.bat };

	Limits lim{ .global_transition_limit = 2000, .global_cost_limit = 5000,
		.stage_transition_limit = 100, .stage_cost_limit = 300, .fairness_limit = 20 };

	for (auto _ : state) {
		scs::BasicActionTheory global = CombineBATs(bats, cm, rm);
		CompleteTopology topology(&graphs);
		GS gs(graphs, graph_recipe, global, topology, lim, shuffling_, rng_);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}

BENCHMARK_DEFINE_F(HingeScale, ManyR)(benchmark::State& state) {
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;

	//
	auto resource1 = HingeGroundedResource1();
	graphs.emplace_back(HingeGroundedResource1Cg());

	auto resource2 = HingeGroundedResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);

	auto resource3 = HingeGroundedResource3();
	graphs.emplace_back(resource3.program, ProgramType::Resource);

	auto resource4 = HingeGroundedResource4();
	graphs.emplace_back(HingeGroundedResource4Cg());

	for (size_t i = 0; i < state.range(0); ++i) {
		graphs.emplace_back(resource2.program, ProgramType::Resource);
	}

	auto recipe_prog = HingeExtendedGroundedRecipe();
	graph_recipe = CharacteristicGraph(recipe_prog, ProgramType::Recipe);
	//

	std::vector<scs::BasicActionTheory> bats{ common_bat, resource1.bat, resource2.bat, resource3.bat, resource4.bat };

	Limits lim{ .global_transition_limit = 2000, .global_cost_limit = 5000,
		.stage_transition_limit = 100, .stage_cost_limit = 300, .fairness_limit = 20 };

	for (auto _ : state) {
		scs::BasicActionTheory global = CombineBATs(bats, cm, rm);
		CompleteTopology topology(&graphs);
		GS gs(graphs, graph_recipe, global, topology, lim, shuffling_, rng_);

		auto controller = gs.Synthethise();
		benchmark::DoNotOptimize(controller);
		benchmark::ClobberMemory();
	}
}


BENCHMARK_REGISTER_F(HingeScale, 2R)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, 3R)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, 4R)->Unit(benchmark::kMillisecond);

/*
	* @Note: ManyR is 4 + arg number
*/
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(1)->Unit(benchmark::kMillisecond)->Iterations(1);
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(2)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(3)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(4)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(5)->Unit(benchmark::kMillisecond);
// BENCHMARK_REGISTER_F(HingeScale, ManyR)->Arg(6)->Unit(benchmark::kMillisecond);


