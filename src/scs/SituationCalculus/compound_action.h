#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

namespace scs {

	// Bend the definition of compound for easier decompositions
	enum class CompoundType {
		Simple = 0,
		Simultaneous = 1,
		Sequence = 2,
	};

	struct CompoundAction {
	private:
		std::vector<Action> actions_;
		CompoundType type_;
	public:
		CompoundAction(const std::vector<Action>& actions) : actions_(actions) {}
		CompoundAction(std::vector<Action>&& actions) : actions_(std::move(actions)) {}
		CompoundAction(const Action& act) {
			actions_.emplace_back(act);
		}

		const std::vector<Action>& Actions() const { return actions_; }
		bool IsSimple() const {
			return actions_.size() == 1;
		}
	};



	inline std::ostream& operator<< (std::ostream& stream, const CompoundAction& ca) {
		stream << "{";
		for (const auto& act : ca.Actions()) {
			stream << act;
		}
		stream << "}";
		return stream;
	}


	inline std::ostream& operator<< (std::ostream& os, const std::vector<CompoundAction>& ca_vec) {
		os << "[Decomposition] \n";
		for (const auto& act : ca_vec) {
			os << '\t' << act;
			os << '\n';
		}
		return os;
	}


}