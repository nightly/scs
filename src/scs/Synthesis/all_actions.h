#pragma once

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/fol.h"

namespace scs {


	struct ActionInstantiations {
	private:
		std::unordered_map<scs::Action, std::vector<scs::Action>> map_;
		const std::unordered_set<Object>* objects_;
	public:
		ActionInstantiations(std::unordered_set<Object>& objects) : objects_(&objects) {}
	
		const std::vector<scs::Action>& Get(const scs::Action& abstract_action) {
			if (map_.contains(abstract_action)) {
				return map_[abstract_action];
			} else {
				ExpandAbstractAction(abstract_action);
				return map_[abstract_action];
			}

		}
	private:
		void ExpandAbstractAction(const scs::Action& abstract_action) {
			std::vector<scs::Action> ret;

			for (const auto& term : abstract_action.terms) {
				if (auto p = std::get_if<Variable>(&term)) {

				}
			}
			
			map_[abstract_action] = ret;
		}
	};


}