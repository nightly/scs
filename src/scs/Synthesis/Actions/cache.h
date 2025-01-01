#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/Combinatorics/CartesianProduct/product.h"
#include "scs/Combinatorics/Actions/instantiations.h"
#include "scs/Synthesis/Topology/types.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	// Handles map of [abstract compound actions -> concrete actions] based on the active domain
	// Also handles relational fluents -> actions (exploiting Markovian property)
	struct Cache {
	private:
		template <typename Key>
		using u_set = ankerl::unordered_dense::set<Key>;

		template <typename Key, typename Value>
		using u_map = ankerl::unordered_dense::map<Key, Value>;
	private:
		// Ungrounded compound actions -> grounded compound actions cache
		u_map<CompoundAction, std::vector<CompoundAction>> actions_cache_;
		
		// Relational fluents -> grounded compound actions cache
		// u_map<Situation, std::vector<CompoundAction>> situation_cache_;

		ActionInstantiations simple_instantiations_;
		const u_set<Object>* objects_;
	public:
		Cache(const u_set<Object>& objects);

		auto& SimpleExecutor() { return simple_instantiations_; }

		const std::vector<CompoundAction>& Get(const CompoundAction& abstract_ca);

		size_t SizeComplete() const; // Size of keys + size of vectors for each key
		size_t SizeSimpleActions() const { return simple_instantiations_.Size(); }
	private:
		Action Flag();

		void Expand(const CompoundAction& abstract_ca);

	public:
		friend std::ostream& operator<< (std::ostream& stream, const Cache& ca_cache);
	};

	std::ostream& operator<< (std::ostream& os, const Cache& ca_cache);

}