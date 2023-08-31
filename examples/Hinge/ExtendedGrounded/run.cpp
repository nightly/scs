#pragma once

#include "recipe.h"
#include "Hinge/ExtendedGrounded/resource_1.h"
#include "Hinge/ExtendedGrounded/resource_2.h"
#include "Hinge/ExtendedGrounded/resource_3.h"
#include "Hinge/ExtendedGrounded/resource_4.h"
#include "Hinge/common.h"

#include "scs/Common/timer.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/synthesis.h"
#include "scs/Common/windows.h"
#include "execution_type.h"

namespace scs::examples {

	void RunHingeExtendedGrounded(const ExecutionType& exec, size_t recipe_variant, bool just_export, bool shuffling, const std::mt19937& rng) {
		std::string dir;
		if (exec == ExecutionType::AStar) {
			dir = "Hinge/ExtendedGrounded/AStar/";
		} else if (exec == ExecutionType::GS) {
			dir = "Hinge/ExtendedGrounded/GS/";
		}

		// ------- Load BATs, Cg --------
		std::vector<CharacteristicGraph> graphs;
		auto common = HingeCommon();
		auto common_bat = HingeCommonBAT();

		auto resource1 = HingeGroundedResource1();
		graphs.emplace_back(resource1.program, ProgramType::Resource);
		ExportGraph(graphs.back(), dir + "Resource1");

		auto resource2 = HingeGroundedResource2();
		graphs.emplace_back(resource2.program, ProgramType::Resource);
		ExportGraph(graphs.back(), dir + "Resource2");

		auto resource3 = HingeGroundedResource3();
		graphs.emplace_back(resource3.program, ProgramType::Resource);
		ExportGraph(graphs.back(), dir + "Resource3");

		auto resource4 = HingeGroundedResource4();
		graphs.emplace_back(resource4.program, ProgramType::Resource);
		ExportGraph(graphs.back(), dir + "Resource4");

		auto recipe_prog = HingeExtendedGroundedRecipe(recipe_variant);
		CharacteristicGraph graph_recipe(recipe_prog, ProgramType::Recipe);
		ExportGraph(graph_recipe, dir + "Recipe");

		// ------------------------------

		// ----- Coop & Routes -----
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

		// -------------------------

		// ------ Global BAT -------
		std::vector<scs::BasicActionTheory> bats{common_bat, resource1.bat, resource2.bat, resource3.bat, resource4.bat};
		auto global = CombineBATs(bats, cm, rm);
		// -------------------------

		// std::cout << common.within_reach;
		// std::cout << global.successors["part"].Form() << std::endl;
		// std::cout << global.pre["In"].Form() << std::endl;

		// std::cout << global;
		// std::cout << global.Initial();
		// std::cout << *graphs[3].lts.states().at(2).key_;
		// -------------------------------

		Timer topology_timer("Topology time");
		CompleteTopology topology(&graphs, false);
		topology_timer.StopWithWrite();

		std::optional<scs::Candidate> controller;
		if (!just_export) {
			if (exec == ExecutionType::AStar) {
				Limits lim{ .global_transition_limit = 50, .global_cost_limit = 200,
					.stage_transition_limit = 5, .stage_cost_limit = 50, .fairness_limit = 20 };
				AStar best(graphs, graph_recipe, global, topology, lim);
				controller = best.Synthethise();
			} else if (exec == ExecutionType::GS) {
				Limits lim{ .global_transition_limit = 50, .global_cost_limit = 200,
					.stage_transition_limit = 12, .stage_cost_limit = 50, .fairness_limit = 20 };
				GS gs(graphs, graph_recipe, global, topology, lim, shuffling, rng);
				controller = gs.Synthethise();
			}
		} else {
			std::cout << global;
		}
		if (controller.has_value()) {
			ExportController(controller.value().plan, dir + "Controller");
		}

		ExportTopology(topology, dir + "Topology");
		if (exec == ExecutionType::AStar) {
			GenerateImagesFromDot("../../exports/Hinge/ExtendedGrounded/AStar/");
		} else if (exec == ExecutionType::GS) {
			GenerateImagesFromDot("../../exports/Hinge/ExtendedGrounded/GS/");
		}

	}
}