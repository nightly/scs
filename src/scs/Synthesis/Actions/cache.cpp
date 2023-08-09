#include "cache.h"

namespace scs {

	CompoundActionCache::CompoundActionCache(const ankerl::unordered_dense::set<Object>& objects)
		: objects_(&objects), simple_instantiations_(objects) {}

	const std::vector<CompoundAction>& CompoundActionCache::Get(const CompoundAction& abstract_ca) {
		if (!cache_.contains(abstract_ca)) {
			Expand(abstract_ca);
		}
		return cache_.at(abstract_ca);
	}

	Action CompoundActionCache::Flag() {
		// This should never be called in practice aside from the initialisation,
		// as we don't expect empty vectors of simple instantiations
		Action a;
		a.name = "EmptyAction";
		return a;
	}

	void CompoundActionCache::Expand(const CompoundAction& abstract_ca) {
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

	/** Utils **/
	std::ostream& operator<< (std::ostream& os, const CompoundActionCache& ca_cache) {
		for (const auto& [k, v] : ca_cache.cache_) {
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

	size_t CompoundActionCache::SizeComplete() const {
		size_t total = 0;
		total += cache_.size();
		for (const auto& [k, v] : cache_) {
			total += v.size();
		}
		return total;
	}


}