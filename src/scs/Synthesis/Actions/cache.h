#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/Combinatorics/CartesianProduct/product.h"
#include "scs/Combinatorics/Actions/instantiations.h"
#include "scs/Synthesis/Topology/types.h"
#include "scs/SituationCalculus/object_universe.h"

#include "ankerl/unordered_dense.h"

namespace scs {
	class BasicActionTheory;

	struct FluentStateHash {
		size_t operator()(const Situation::u_map<std::string, RelationalFluent>& fluents) const;
	};

	// Handles map of [abstract compound actions -> concrete actions] based on the active domain
	// Also handles relational fluents -> actions (exploiting Markovian property)
	struct Cache {
	private:
		template <typename Key>
		using u_set = ankerl::unordered_dense::set<Key>;

		template <typename Key, typename Value, typename Hash = std::hash<Key>>
		using u_map = ankerl::unordered_dense::map<Key, Value, Hash>;

		struct EvaluationKey {
			CompoundAction action;
			std::vector<Object> objects;

			bool operator==(const EvaluationKey& other) const {
				return action == other.action && objects == other.objects;
			}
		};

		struct EvaluationKeyHash {
			size_t operator()(const EvaluationKey& key) const;
		};

		struct SituationCacheEntry {
			u_map<EvaluationKey, bool, EvaluationKeyHash> possible_actions;
			u_map<EvaluationKey, Situation, EvaluationKeyHash> successors;
		};
	private:
		// Ungrounded compound actions -> grounded compound actions cache
		u_map<CompoundAction, std::vector<CompoundAction>> actions_cache_;
		
		// Complete fluent state -> action executability and deterministic successor state.
		ankerl::unordered_dense::map<Situation::u_map<std::string, RelationalFluent>,
			SituationCacheEntry, FluentStateHash> situation_cache_;
		size_t situation_cache_hits_ = 0;

		ActionInstantiations simple_instantiations_;
		const u_set<Object>* objects_;
	public:
		Cache(const u_set<Object>& objects);

		const std::vector<CompoundAction>& Get(const CompoundAction& abstract_ca);
		bool Possible(const Situation& situation, const CompoundAction& action,
			const BasicActionTheory& bat, bool markovian_situations,
			const ObjectSet* objects = nullptr);
		Situation Progress(const Situation& situation, const CompoundAction& action,
			const BasicActionTheory& bat, bool markovian_situations,
			const ObjectSet* objects = nullptr);

		size_t SizeComplete() const; // Size of keys + size of vectors for each key
		size_t SizeSimpleActions() const { return simple_instantiations_.Size(); }
		size_t SizeSituationStates() const { return situation_cache_.size(); }
		size_t SituationCacheHits() const { return situation_cache_hits_; }
	private:
		Action Flag();

		void Expand(const CompoundAction& abstract_ca);
		EvaluationKey MakeEvaluationKey(const Situation& situation, const CompoundAction& action,
			const BasicActionTheory& bat, const ObjectSet* objects) const;

	public:
		friend std::ostream& operator<< (std::ostream& stream, const Cache& ca_cache);
	};

	std::ostream& operator<< (std::ostream& os, const Cache& ca_cache);

}
