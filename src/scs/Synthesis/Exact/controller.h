#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "scs/Synthesis/Exact/solver.h"

namespace scs {

	using FreshIdentifierProvider = std::function<Object(const ObjectSet&)>;

	FreshIdentifierProvider SequentialFreshIdentifiers(std::string prefix = "fresh-");

	struct ControllerResponse {
		std::vector<JointAction> actions;
		uint64_t cost = 0;
		bool reached_goal = false;
	};

	struct RecipeEdgeChoice {
		CompoundAction request;
		FirstOrderAssignment bindings;
		// Required when request and retained bindings do not identify one residual.
		std::optional<size_t> target_control;
	};

	class ControllerSession {
	public:
		ControllerSession(const SynthesisProblem& problem, const Controller& controller,
			FreshIdentifierProvider fresh_identifiers,
			Interpretation concrete_initial = {});

		ControllerResponse Respond(const RecipeEdgeChoice& concrete_choice);
		ControllerResponse Respond(const CompoundAction& concrete_request);
		void Stop();

		ArenaStateId abstract_state() const { return abstract_state_; }
		const Interpretation& concrete_state() const { return concrete_state_; }
		uint64_t response_budget() const { return response_budget_; }

	private:
		const SynthesisProblem* problem_ = nullptr;
		const Controller* controller_ = nullptr;
		FreshIdentifierProvider fresh_identifiers_;
		ArenaStateId abstract_state_ = 0;
		Interpretation concrete_state_;
		ObjectRenaming representative_to_concrete_;
		ObjectSet used_identifiers_;
		uint64_t response_budget_ = 0;
		std::optional<CompoundAction> pending_request_;

		std::optional<ObjectRenaming> MatchRequest(const CompoundAction& representative,
			const CompoundAction& concrete) const;
		JointAction Lift(const JointAction& representative);
		void UpdateMappingForTarget(const ArenaEdge& edge, bool allocate_missing);
	};

}
