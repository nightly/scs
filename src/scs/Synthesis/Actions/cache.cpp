#include "cache.h"

#include <boost/container_hash/hash.hpp>

#include "scs/SituationCalculus/bat.h"

namespace scs {

	size_t FluentStateHash::operator()(const Situation::u_map<std::string, RelationalFluent>& fluents) const {
		size_t entries_hash = 0;
		for (const auto& [name, fluent] : fluents) {
			size_t entry_hash = std::hash<std::string>{}(name);
			boost::hash_combine(entry_hash, RelationalFluentHash{}(fluent));
			entries_hash += entry_hash;
		}

		size_t seed = 0;
		boost::hash_combine(seed, fluents.size());
		boost::hash_combine(seed, entries_hash);
		return seed;
	}

	Cache::Cache(const ankerl::unordered_dense::set<Object>& objects)
		: simple_instantiations_(objects), objects_(&objects) {}

	const std::vector<CompoundAction>& Cache::Get(const CompoundAction& abstract_ca) {
		if (!actions_cache_.contains(abstract_ca)) {
			Expand(abstract_ca);
		}
		return actions_cache_.at(abstract_ca);
	}

	bool Cache::Possible(const Situation& situation, const CompoundAction& action,
		const BasicActionTheory& bat, bool markovian_situations) {
		if (!markovian_situations) {
			return situation.Possible(action, bat);
		}

		auto state = situation_cache_.try_emplace(situation.Fluents()).first;
		auto possible = state->second.possible_actions.find(action);
		if (possible != state->second.possible_actions.end()) {
			++situation_cache_hits_;
			return possible->second;
		}

		const bool result = situation.Possible(action, bat);
		state->second.possible_actions.emplace(action, result);
		return result;
	}

	Situation Cache::Progress(const Situation& situation, const CompoundAction& action,
		const BasicActionTheory& bat, bool markovian_situations) {
		if (!markovian_situations) {
			return situation.Do(action, bat);
		}

		auto state = situation_cache_.try_emplace(situation.Fluents()).first;
		auto successor = state->second.successors.find(action);
		if (successor != state->second.successors.end()) {
			++situation_cache_hits_;
			return successor->second;
		}

		Situation next = situation.Do(action, bat, true);
		state->second.successors.emplace(action, next);
		return next;
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
