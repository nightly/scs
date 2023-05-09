#pragma once

namespace scs {
	
	struct Execution {
		bool is_iteration = false;



	};

}

/*
	inline std::ostream& operator<< (std::ostream& os, const CompoundAction& ca) {
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


	inline std::ostream& operator<< (std::ostream& os, const std::vector<CompoundAction>& ca_vec) {
		os << "[Decomposition] \n";
		for (const auto& act : ca_vec) {
			os << '\t' << act;
			os << '\n';
		}
		return os;
	}

*/