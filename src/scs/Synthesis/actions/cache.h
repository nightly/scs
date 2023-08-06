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
		CompoundActionCache(const ankerl::unordered_dense::set<Object>& objects)
			: objects_(&objects), simple_instantiations_(objects) {}


		auto& SimpleExecutor() { return simple_instantiations_; }

		const std::vector<CompoundAction>& Get(const CompoundAction& abstract_ca) {
			if (!cache_.contains(abstract_ca)) {
				Expand(abstract_ca);
			}
			return cache_.at(abstract_ca);
		}

		Action Flag() {
			Action a;
			a.name = "EmptyAction";
			return a;
		}

		void Expand(const CompoundAction& abstract_ca) {
			assert(!cache_.contains(abstract_ca) && "Expanding abstract CA multiple times?");
			// Get all instantiations of simple actions as a vector
			std::vector<const std::vector<Action>*> vec;
			for (const auto& simple : abstract_ca.Actions()) {
				const auto& a_x = simple_instantiations_.Get(simple);
				vec.emplace_back(&a_x);
			}

			// Calculate cartesian product of simple action vectors
			auto prod = Product(vec, Flag());
			std::vector<CompoundAction> concrete_actions;
			for (auto& acts : prod) {
				// For each product found, construct CompoundAction, 
				// store in a vector which is the value of the abstract_ca key in map
				CompoundAction concrete_ca;
				concrete_ca.SetActions(acts);
				concrete_actions.emplace_back(std::move(concrete_ca));
			}
			cache_[abstract_ca] = std::move(concrete_actions);
		}

	};


}