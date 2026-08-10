#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "scs/Synthesis/Exact/model.h"
#include "scs/Synthesis/Topology/types.h"

namespace scs {

	using ArenaStateId = size_t;
	using ArenaEdgeId = size_t;

	enum class ArenaOwner {
		Environment,
		Controller,
		Goal,
		Lose,
	};

	struct ArenaState {
		ArenaOwner owner = ArenaOwner::Environment;
		CgState recipe_control;
		FirstOrderAssignment recipe_bindings;
		TopologyState facility_control;
		std::vector<FirstOrderAssignment> facility_bindings;
		Interpretation interpretation;
		std::optional<CompoundAction> pending;

		bool operator==(const ArenaState&) const = default;
	};

	enum class AuxiliaryLabel {
		Stop,
		Dead,
		Sink,
	};

	using ArenaLabel = std::variant<CompoundAction, JointAction, AuxiliaryLabel>;
	using ObjectRenaming = std::vector<std::pair<Object, Object>>;

	struct ArenaEdge {
		ArenaStateId source = 0;
		ArenaStateId target = 0;
		ArenaLabel label = AuxiliaryLabel::Sink;
		uint64_t cost = 0;
		ObjectRenaming witness;
	};

	struct ArenaBounds {
		size_t active_domain = 0;
		size_t recipe_live = 0;
		size_t facility_live = 0;
		size_t recipe_action_arity = 0;
		size_t support = 0;
		size_t fresh_edge = 0;
		size_t pool = 0;
	};

	struct ArenaStatistics {
		size_t generated_substitutions = 0;
		size_t progression_calls = 0;
		size_t isomorphism_checks = 0;
		size_t rejected_guards = 0;
		size_t rejected_preconditions = 0;
		size_t rejected_observations = 0;
		size_t dominated_edges = 0;
		size_t removed_self_loops = 0;
		size_t callback_contract_samples = 0;
	};

	struct Arena {
		std::vector<ArenaState> states;
		std::vector<ArenaEdge> edges;
		std::vector<std::vector<ArenaEdgeId>> outgoing;
		ArenaStateId initial = 0;
		ArenaStateId goal = 0;
		ArenaStateId lose = 0;
		ArenaBounds bounds;
		ArenaStatistics statistics;

		ArenaStateId AddState(ArenaState state);
		ArenaEdgeId AddEdge(ArenaEdge edge);
	};

	enum class ArenaBuildStatus {
		Complete,
		InvalidModel,
		Cancelled,
	};

	struct ArenaBuildResult {
		ArenaBuildStatus status = ArenaBuildStatus::InvalidModel;
		Arena arena;
		std::vector<std::string> diagnostics;
	};

	ArenaBounds ComputeArenaBounds(const SynthesisProblem& problem,
		const CharacteristicGraph& recipe,
		const std::vector<CharacteristicGraph>& resources,
		size_t active_domain_bound);
	ObjectSet StateSupport(const ArenaState& state);
	ObjectSet EdgeSupport(const ArenaState& source, const ArenaLabel& label,
		const ArenaState& target);
	ArenaState RenameState(const ArenaState& state, const ObjectRenaming& renaming);
	ArenaLabel RenameLabel(const ArenaLabel& label, const ObjectRenaming& renaming);
	bool IsBijectionWitness(const ObjectRenaming& witness);

	ArenaBuildResult BuildArena(const SynthesisProblem& problem,
		const SynthesisOptions& options);

}

namespace std {
	template <>
	struct hash<scs::ArenaState> {
		size_t operator()(const scs::ArenaState& state) const;
	};
}
