#pragma once

#include <vector>
#include <unordered_map>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/Combinatorics/CartesianProduct/product.h"
#include "scs/Combinatorics/Actions/instantiations.h"
#include "scs/Synthesis/Topology/types.h"

namespace scs {

	struct CompoundActionCache {
	private:
		// Ground abstract compound action action to all concrete actions
		std::unordered_map<CompoundAction, std::vector<CompoundAction>> cache_; 
		
		ActionInstantiations simple_instantiations_;
		const std::unordered_set<Object>* objects_;
	public:
		CompoundActionCache(const std::unordered_set<Object>& objects) 
			: objects_(&objects), simple_instantiations_(objects) {}


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
			// Get all instantiations of simple actions as a vector
			std::vector<const std::vector<Action>*> vec;
			for (const auto& simple : abstract_ca.Actions()) {
				const auto& a_x = simple_instantiations_.Get(abstract_ca.Actions().at(0));
				vec.emplace_back(&a_x);
			}

			// Calculate cartesian product of simple action vectors
			auto prod = Product(vec, Flag());
			auto& key_val_vec = cache_[abstract_ca];
			assert(key_val_vec.empty() && "Compound action key should be empty before expanding its instantiations");
			for (auto& acts : prod) {
				// For each product found, construct CompoundAction, 
				// store in a vector which is the value of the abstract_ca key in map
				CompoundAction concrete_ca;
				concrete_ca.SetActions(acts);
				key_val_vec.emplace_back(std::move(concrete_ca));
			}
		}

	};


}