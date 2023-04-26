#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

namespace scs {

	// Bend the definition of compound for easier decompositions
	enum class ActionStateType {
		Simple = 0,
		Simultaneous = 1,
		Sequence = 2,
	};

	struct ActionState {
	private:
		std::vector<Action> actions_;
		ActionStateType type_;
	public:
		ActionState(const std::vector<Action>& actions, ActionStateType type) : actions_(actions), type_(type) {}
		ActionState(std::vector<Action>&& actions, ActionStateType type) : actions_(std::move(actions)), type_(type) {}

		ActionState(const Action& act) {
			actions_.emplace_back(act);
		}
		ActionState(const Action& act, ActionStateType type) {
			actions_.emplace_back(act);
			type_ = type;
		}		
		ActionState(Action&& act, ActionStateType type) {
			actions_.emplace_back(std::move(act));
			type_ = type;
		}

		const std::vector<Action>& Actions() const { return actions_; }
		const ActionStateType Type() const { return type_;  }
		bool IsSimple() const {
			return actions_.size() == 1;
		}
	};



	inline std::ostream& operator<< (std::ostream& os, const ActionState& ca) {
		switch (ca.Type()) {
		case ActionStateType::Simple:
			os << "<Simple> ";
			break;

		case ActionStateType::Simultaneous:
			os << "<Sim> ";
			break;

		case ActionStateType::Sequence:
			os << "<Sequence> ";
			break;

		default:
			os << "<Some Type> ";
			break;
		
		}

		os << "{";
		for (const auto& act : ca.Actions()) {
			os << act;
		}
		os << "}";
		return os;
	}


	inline std::ostream& operator<< (std::ostream& os, const std::vector<ActionState>& ca_vec) {
		os << "[Decomposition] \n";
		for (const auto& act : ca_vec) {
			os << '\t' << act;
			os << '\n';
		}
		return os;
	}


}