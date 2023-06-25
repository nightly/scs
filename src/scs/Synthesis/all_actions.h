#pragma once

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"

namespace scs {


	struct ActionInstantiations {
	private:
		std::unordered_map<scs::Action, std::vector<scs::Action>> map_;
		std::vector<std::string> objects_;

		std::unordered_map<size_t, std::vector<std::vector<std::string>>> permutations_;
	public:
		ActionInstantiations(std::unordered_set<Object>& objects_set) {
			objects_.reserve(objects_set.size());
			objects_.insert(objects_.end(), objects_set.begin(), objects_set.end());
		}
	
		const std::vector<scs::Action>& Get(const scs::Action& abstract_action) {
			if (map_.contains(abstract_action)) {
				return map_[abstract_action];
			} else {
				ExpandAbstractAction(abstract_action);
				return map_[abstract_action];
			}

		}
	private:
		std::vector<scs::Action> GenerateInstantiations(const scs::Action& abstract_action, 
		const std::vector<std::vector<std::string>>& permutations) {
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

		void ExpandAbstractAction(const scs::Action& abstract_action) {
			size_t r = 0;
			for (const auto& term : abstract_action.terms) {
				if (auto p = std::get_if<Variable>(&term)) {
					r++;
				}
			}
			if (!permutations_.contains(r)) {
				permutations_[r] = GeneratePermutations(r);
			}

			auto ret = GenerateInstantiations(abstract_action, permutations_[r]);

			map_[abstract_action] = ret;
		}

		std::vector<std::vector<std::string>> GeneratePermutations(size_t n) {
			std::vector<std::vector<std::string>> permutations;
			std::vector<std::string> current;
			std::vector<bool> used(objects_.size(), false);
			GenPermutations(n, current, used, permutations);
			return permutations;
		}

		void GenPermutations(size_t n, std::vector<std::string>& current, std::vector<bool>& used, 
		std::vector<std::vector<std::string>>& permutations) {
			if (current.size() == n) {
				permutations.push_back(current);
				return;
			}

			for (size_t i = 0; i < objects_.size(); i++) {
				if (!used[i]) {
					used[i] = true;
					current.push_back(objects_[i]);
					GenPermutations(n, current, used, permutations);
					used[i] = false;
					current.pop_back();
				}
			}
		}
	};


}