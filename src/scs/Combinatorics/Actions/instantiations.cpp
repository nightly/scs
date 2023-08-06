#include "scs/Combinatorics/Actions/instantiations.h"

#include <vector>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	// Constructor that converts object_ uset to vector
	// We could just iterate over the uset...
	ActionInstantiations::ActionInstantiations(const ankerl::unordered_dense::set<Object>& objects_set) {
		objects_.reserve(objects_set.size());
		objects_.insert(objects_.end(), objects_set.begin(), objects_set.end());
	}

	// Cache lookup if already grounded for the particular abstract action type
	const std::vector<scs::Action>& ActionInstantiations::Get(const scs::Action& abstract_action) {
		if (!map_.contains(abstract_action)) {
			ExpandAbstractAction(abstract_action);
		}
		return map_.at(abstract_action);
	}

	void ActionInstantiations::ExpandAbstractAction(const scs::Action& abstract_action) {
		Permutation perm;
		perm.r = 0;
		perm.used.reserve(objects_.size());
		for (const auto& term : abstract_action.terms) {
			if (auto p = std::get_if<Variable>(&term)) {
				perm.r++;
			} else if (auto p = std::get_if<Object>(&term)) {
				perm.used.emplace(*p);
			}
		}
		const auto& perms_vec = FetchPermutation(perm);
		auto ret = PlaceInstantiations(abstract_action, perms_vec);
		map_[abstract_action] = std::move(ret);
	}

	const std::vector<std::vector<Object>>& ActionInstantiations::FetchPermutation(Permutation& perm) {
		// Useful in NextStages() hence why public
		if (!permutations_cache_.contains(perm)) {
			permutations_cache_[perm] = ExpandPermutation(perm.r, perm.used);
		}
		return permutations_cache_[perm];
	}


	// Place instantiations from permutations
	std::vector<scs::Action> ActionInstantiations::PlaceInstantiations(const scs::Action& abstract_action,
	const std::vector<std::vector<Object>>& permutations) {
		std::vector<scs::Action> instantiations;
		for (const auto& perm : permutations) {
			scs::Action current = abstract_action;
			size_t n = 0;
			for (auto& term : current.terms) {
				if (auto ptr = std::get_if<Variable>(&term)) {
					term = scs::Object(perm[n]);
					n++;
				}
			}
			instantiations.emplace_back(current);
		}
		return instantiations;
	}


	// Initialise recursive search
	std::vector<std::vector<Object>> ActionInstantiations::ExpandPermutation(size_t n, ankerl::unordered_dense::set<Object>& used) {
		std::vector<std::vector<Object>> permutations;
		std::vector<Object> current;
		GenPermutations(n, current, used, permutations);
		return permutations;
	}

	// Recursively find all permutations
	void ActionInstantiations::GenPermutations(size_t n, std::vector<Object>& current, ankerl::unordered_dense::set<Object>& used,
	std::vector<std::vector<Object>>& permutations) {
		if (current.size() == n) {
			permutations.emplace_back(current);
			return;
		}

		for (size_t i = 0; i < objects_.size(); i++) {
			if (!used.contains(objects_[i])) {
				used.emplace(objects_[i]);
				current.emplace_back(objects_[i]);
				GenPermutations(n, current, used, permutations);
				used.erase(objects_[i]);
				current.pop_back();
			}
		}
	}

	/** Utils **/
	size_t ActionInstantiations::Cardinality(const scs::Action& abstract_action) {
		const auto& got = Get(abstract_action);
		return got.size();
	}
	static size_t Factorial(size_t n) {
		return (n == 0) || (n == 1) ? 1 : n * Factorial(n - 1);
	}
	// @param already_used: if X objects are mentioned as constants and there needs to be R pickings,
	// the space is = (space - X) as objects can't be duplicated
	size_t ActionInstantiations::ExpectedCardinality(size_t r, size_t space, size_t already_used) const {
		size_t ret = 0;
		space = space - already_used;
		// n! / (n-r)!
		return Factorial(r) / Factorial(space - r);
	}
}