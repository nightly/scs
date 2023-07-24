#pragma once

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"

namespace scs {

	// 
	// @Todo:
	// - [ ] write benchmark, particularly for bigger cases
	// - [ ] operate on scs::Object directly not std::strings
	// - [ ] use flat_map/flat_sets instead of converting to vector
	// - [ ] cache Permutations as well as action types themselves, because they could be reused if:
	//		- r is the same
	//		- the objects used are the same
	//

	struct Permutation {
		// used set
		// r (choice)
	};

	struct ActionInstantiations {
	private:
		std::unordered_map<scs::Action, std::vector<scs::Action>> map_;
		std::vector<std::string> objects_;
	public:
		ActionInstantiations(const std::unordered_set<Object>& objects_set);
		const std::vector<scs::Action>& Get(const scs::Action& abstract_action);
	private:
		void ExpandAbstractAction(const scs::Action& abstract_action);
		void MarkUsed(std::vector<bool>& used, const Object& o) const;
		std::vector<std::vector<std::string>> ExpandPermutation(size_t n, std::vector<bool>& used);

		void GenPermutations(size_t n, std::vector<std::string>& current, std::vector<bool>& used,
			std::vector<std::vector<std::string>>& permutations);
	private:
		static std::vector<scs::Action> PlaceInstantiations(const scs::Action& abstract_action,
			const std::vector<std::vector<std::string>>& permutations);
	};


}