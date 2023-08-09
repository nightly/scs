#pragma once

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"
#include "scs/Combinatorics/Actions/permutation.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	struct ActionInstantiations {
	private:
		std::unordered_map<scs::Action, std::vector<scs::Action>> map_;
		std::vector<scs::Object> objects_;

		ankerl::unordered_dense::map<Permutation, std::vector<std::vector<Object>>> permutations_cache_;
	public:
		ActionInstantiations(const ankerl::unordered_dense::set<Object>& objects_set);
		const std::vector<scs::Action>& Get(const scs::Action& abstract_action);
		const std::vector<std::vector<Object>>& FetchPermutation(Permutation& perm); // Private method, but useful elsewhere (stages)

		size_t Size() const;
	private:
		void ExpandAbstractAction(const scs::Action& abstract_action);
		std::vector<std::vector<Object>> ExpandPermutation(size_t n, ankerl::unordered_dense::set<Object>& used);

		void GenPermutations(size_t n, std::vector<Object>& current, ankerl::unordered_dense::set<Object>& used,
			std::vector<std::vector<Object>>& permutations);
	public:
		size_t Cardinality(const scs::Action& abstract_action);
		size_t ExpectedCardinality(size_t r, size_t space, size_t already_used) const;

		friend std::ostream& operator<< (std::ostream& os, const ActionInstantiations& inst);
	private:
		static std::vector<scs::Action> PlaceInstantiations(const scs::Action& abstract_action,
			const std::vector<std::vector<Object>>& permutations);
	};

	std::ostream& operator<< (std::ostream& os, const ActionInstantiations& inst);

}