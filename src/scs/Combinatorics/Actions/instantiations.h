#pragma once

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"
#include "scs/Combinatorics/Actions/permutation.h"

#include "boost/container/flat_map.hpp"
#include "boost/container/flat_set.hpp"

namespace scs {

	// 
	// @Todo:
	// - [ ] use flat_map/flat_sets instead of converting to vector
	// - [ ] cache Permutations as well as action types themselves, because they could be reused if:
	//		- r is the same
	//		- the objects used are the same

	struct ActionInstantiations {
	private:
		std::unordered_map<scs::Action, std::vector<scs::Action>> map_;
		std::vector<scs::Object> objects_;
	public:
		ActionInstantiations(const std::unordered_set<Object>& objects_set);
		const std::vector<scs::Action>& Get(const scs::Action& abstract_action);
		size_t Cardinality(const scs::Action& abstract_action);
		size_t ExpectedCardinality(size_t r, size_t space, size_t already_used) const;
	private:
		void ExpandAbstractAction(const scs::Action& abstract_action);
		void MarkUsed(std::vector<bool>& used, const Object& o) const;
		std::vector<std::vector<Object>> ExpandPermutation(size_t n, std::vector<bool>& used);

		void GenPermutations(size_t n, std::vector<Object>& current, std::vector<bool>& used,
			std::vector<std::vector<Object>>& permutations);
	private:
		static std::vector<scs::Action> PlaceInstantiations(const scs::Action& abstract_action,
			const std::vector<std::vector<Object>>& permutations);
	};


}