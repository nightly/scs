#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/Combinatorics/CartesianProduct/product.h"
#include "scs/Combinatorics/Actions/instantiations.h"
#include "scs/Synthesis/Topology/types.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	struct CompoundActionCache {
	private:
		// Ground abstract compound action action to all concrete actions
		ankerl::unordered_dense::map<CompoundAction, std::vector<CompoundAction>> cache_;
		
		ActionInstantiations simple_instantiations_;
		const ankerl::unordered_dense::set<Object>* objects_;
	public:
		CompoundActionCache(const ankerl::unordered_dense::set<Object>& objects);

		auto& SimpleExecutor() { return simple_instantiations_; }

		const std::vector<CompoundAction>& Get(const CompoundAction& abstract_ca);

		size_t SizeComplete() const; // Size of keys + size of vectors for each key
		size_t SizeSimpleActions() const { return simple_instantiations_.Size(); }
	private:
		Action Flag();

		void Expand(const CompoundAction& abstract_ca);

	public:
		friend std::ostream& operator<< (std::ostream& stream, const CompoundActionCache& ca_cache);
	};

	std::ostream& operator<< (std::ostream& os, const CompoundActionCache& ca_cache);

}