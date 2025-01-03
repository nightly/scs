#pragma once

#include <vector>
#include <span>

#include "scs/Synthesis/Solvers/Core/candidate.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Synthesis/Solvers/Heuristics/limits.h"
#include "scs/Synthesis/Topology/types.h"
#include "scs/Synthesis/Topology/interface_topology.h"
#include "scs/Combinatorics/Actions/instantiations.h"
// #include "scs/Synthesis/Actions/cache.h";

namespace scs {	

	Candidate CreateInitialCandidate(const BasicActionTheory& bat, const std::span<CharacteristicGraph>& resource_graphs, 
		const ITopology& topology, const CharacteristicGraph& recipe_graph, ActionInstantiations& ai);
	
	size_t TotalRecipeTransitions(const CharacteristicGraph& cg);
	bool Holds(const Stage& stage, const Formula& form, const BasicActionTheory& bat);
	size_t AddControllerTransition(Candidate& candidate, Stage& next_stage, const PlanTransition& trans, const Stage& previous_stage);
	bool WithinLimits(const Candidate& cand, const Stage& stage, const Limits& lim);
	
	void NextStages(Candidate& next_candidate, const Stage& old_stage,
		const CharacteristicGraph& recipe_graph, const BasicActionTheory& bat, const Limits& lim, 
		const TopologyState* next_resources_state, ActionInstantiations& ai);

	void UpdateBest(const Candidate& cand, Candidate& best_candidate);

}