#include "cache.h"

namespace scs {

	Cache::Cache(const ankerl::unordered_dense::set<Object>& objects)
		: objects_(&objects), simple_instantiations_(objects) {}

	const std::vector<CompoundAction>& Cache::Get(const CompoundAction& abstract_ca) {
		if (!actions_cache_.contains(abstract_ca)) {
			Expand(abstract_ca);
		}
		return actions_cache_.at(abstract_ca);
	}

	Action Cache::Flag() {
		// This should never be called in practice aside from the initialisation,
		// as we don't expect empty vectors of simple instantiations
		Action a;
		a.name = "EmptyAction";
		return a;
	}

	void Cache::Expand(const CompoundAction& abstract_ca) {
		assert(!actions_cache_.contains(abstract_ca) && "Expanding abstract CA multiple times?");
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
		actions_cache_[abstract_ca] = std::move(concrete_actions);
	}

	/** Utils **/
	std::ostream& operator<< (std::ostream& os, const Cache& ca_cache) {
		for (const auto& [k, v] : ca_cache.actions_cache_) {
			os << k << " = {";
			for (auto it = v.begin(); it != v.end(); ++it) {
				if (it != v.begin()) {
					os << ", ";
				}
				os << *it;
			}
			os << "}\n";
		}
		return os;
	}

	size_t Cache::SizeComplete() const {
		size_t total = 0;
		for (const auto& [k, v] : actions_cache_) {
			total += v.size();
		}
		return total;
	}

}