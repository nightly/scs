#pragma once

#include <optional>
#include <span>

#include "scs/Synthesis/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/Synthesis/all_actions.h"
#include "scs/Synthesis/unify.h"
#include "scs/Synthesis/valuation.h"
#include "scs/Synthesis/solvers/best/limits.h"
#include "scs/Synthesis/solvers/best/candidate.h"

namespace scs {

	struct Best {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;

		scs::ActionInstantiations action_cache;
		
	public:
		Best(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, const Limits& lim = Limits()) : resource_graphs(resource_graphs), recipe_graph(recipe_graph),
			global_bat(global_bat), lim(lim), action_cache(global_bat.Initial().objects) {}
	
		std::optional<Plan> Synthethise() {


			return std::nullopt;
		}
	
	};
	

}