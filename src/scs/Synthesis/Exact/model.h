#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

#include "scs/ConGolog/resource.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/FirstOrderLogic/domain.h"
#include "scs/SituationCalculus/object_universe.h"

namespace scs {

	struct ResourceStep {
		ResourceIndex resource = 0;
		CompoundAction action;

		bool operator==(const ResourceStep&) const = default;
	};

	struct JointAction {
		std::vector<ResourceStep> steps;

		CompoundAction Flatten() const;
		CompoundAction IndexedFlatten() const;
		bool IsGround() const;
		bool AreAllNop() const;
		bool operator==(const JointAction&) const = default;
	};

	std::string IndexedActionName(ResourceIndex resource, std::string_view action_name);
	Formula ResourceActionOccurs(ResourceIndex resource, const Action& action);
	Formula JointActionMatches(const std::vector<ResourceStep>& steps);

	std::ostream& operator<<(std::ostream& output, const ResourceStep& step);
	std::ostream& operator<<(std::ostream& output, const JointAction& action);

	struct FacilityProgramStateView {
		const std::vector<CgState>* control = nullptr;
		const std::vector<FirstOrderAssignment>* bindings = nullptr;
	};

	using JointExecutability = std::function<bool(const JointAction&, const Interpretation&)>;
	using JointCandidateFilter = std::function<bool(const JointAction&)>;
	using ActionObservation = std::function<std::optional<CompoundAction>(const JointAction&)>;
	using TransitionCost = std::function<uint64_t(const FacilityProgramStateView&,
		const Interpretation&, const JointAction&, const FacilityProgramStateView&,
		const Interpretation&)>;

	struct FacilityCallbacks {
		JointCandidateFilter may_be_possible;
		JointExecutability possible;
		ActionObservation observe;
		TransitionCost cost;
	};

	struct FacilityComposition {
		BasicActionTheory common;
		ankerl::unordered_dense::map<std::string, Successor> combined_successors;
		FacilityCallbacks callbacks;
	};

	struct Facility {
		std::vector<Resource> resources;
		BasicActionTheory bat;
		FacilityCallbacks callbacks;

		bool Possible(const JointAction& action, const Interpretation& state,
			const ObjectSet& objects, DomainSemantics semantics) const;
		std::optional<CompoundAction> Observe(const JointAction& action) const;
		uint64_t Cost(const FacilityProgramStateView& source_control,
			const Interpretation& source, const JointAction& action,
			const FacilityProgramStateView& target_control,
			const Interpretation& target) const;
	};

	Facility ComposeFacility(std::vector<Resource> resources,
		FacilityComposition composition = {});
	std::vector<std::string> ValidateFacility(const Facility& facility);

	struct SynthesisProblem {
		Facility facility;
		std::shared_ptr<IProgram> recipe;
	};

	enum class WorklistOrder {
		BreadthFirst,
		LowerCostFirst,
		Greedy,
	};

	struct FiniteDomainBackend {
		ObjectSet explicit_objects;
	};

	struct FaithfulAbstractionBackend {
		size_t active_domain_bound = 0;
		WorklistOrder worklist_order = WorklistOrder::BreadthFirst;
	};

	using SynthesisBackend = std::variant<FiniteDomainBackend, FaithfulAbstractionBackend>;

	struct SynthesisOptions {
		SynthesisBackend backend = FaithfulAbstractionBackend{};
		std::optional<std::chrono::steady_clock::time_point> deadline;
		bool validate_controller = true;
	};

}

namespace std {
	template <>
	struct hash<scs::ResourceStep> {
		size_t operator()(const scs::ResourceStep& step) const {
			size_t seed = step.resource;
			boost::hash_combine(seed, std::hash<scs::CompoundAction>{}(step.action));
			return seed;
		}
	};

	template <>
	struct hash<scs::JointAction> {
		size_t operator()(const scs::JointAction& action) const {
			size_t seed = 0;
			for (const auto& step : action.steps) {
				boost::hash_combine(seed, std::hash<scs::ResourceStep>{}(step));
			}
			return seed;
		}
	};
}
