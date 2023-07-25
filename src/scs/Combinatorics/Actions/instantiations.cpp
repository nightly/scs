#include "scs/Combinatorics/Actions/instantiations.h"

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"

namespace scs {

	// Constructor that converts object_ uset to vector
	ActionInstantiations::ActionInstantiations(const std::unordered_set<Object>& objects_set) {
		objects_.reserve(objects_set.size());
		objects_.insert(objects_.end(), objects_set.begin(), objects_set.end());
	}

	// Cache lookup if already grounded for the particular abstract action type
	const std::vector<scs::Action>& ActionInstantiations::Get(const scs::Action& abstract_action) {
		if (map_.contains(abstract_action)) {
			return map_[abstract_action];
		} else {
			ExpandAbstractAction(abstract_action);
			return map_[abstract_action];
		}
	}

	void ActionInstantiations::ExpandAbstractAction(const scs::Action& abstract_action) {
		size_t r = 0;
		std::vector<bool> used(objects_.size(), false);
		for (const auto& term : abstract_action.terms) {
			if (auto p = std::get_if<Variable>(&term)) {
				r++;
			} else if (auto p = std::get_if<Object>(&term)) {
				MarkUsed(used, *p);
			}
		}
		auto perm = ExpandPermutation(r, used);
		auto ret = PlaceInstantiations(abstract_action, perm);
		map_[abstract_action] = ret;
	}

	void ActionInstantiations::MarkUsed(std::vector<bool>& used, const scs::Object& o) const {
		for (size_t i = 0; i < objects_.size(); ++i) {
			if (objects_[i] == o.name()) {
				used[i] = true;
			}
		}
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
	std::vector<std::vector<Object>> ActionInstantiations::ExpandPermutation(size_t n, std::vector<bool>& used) {
		std::vector<std::vector<Object>> permutations;
		std::vector<Object> current;
		GenPermutations(n, current, used, permutations);
		return permutations;
	}

	// Recursively find all permutations
	void ActionInstantiations::GenPermutations(size_t n, std::vector<Object>& current, std::vector<bool>& used,
	std::vector<std::vector<Object>>& permutations) {
		if (current.size() == n) {
			permutations.emplace_back(current);
			return;
		}

		for (size_t i = 0; i < objects_.size(); i++) {
			if (!used[i]) {
				used[i] = true;
				current.emplace_back(objects_[i]);
				GenPermutations(n, current, used, permutations);
				used[i] = false;
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