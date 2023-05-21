#pragma once

#include <vector>
#include <variant>
#include <ostream>

#include "scs/SituationCalculus/action.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {
	
	struct Execution {
	public:
		std::vector<std::variant<Action, Formula>> trace;
		std::vector<Execution> sub_executions; // Note: sub_executions can only be traced at the end of trace.
	public:
		bool RecursiveExecutions() const { return !sub_executions.empty(); }
		friend std::ostream& operator<< (std::ostream& os, const Execution& exec);
	};

	inline void PrintTrace(std::ostream& os, const std::vector<std::variant<Action, Formula>>& trace) {
		for (size_t i = 0; i < trace.size(); ++i) {
			const auto& e = trace[i];

			if (auto act_ptr = std::get_if<Action>(&e)) {
				const auto& act = *act_ptr;
				os << act;
			} else if (auto form_ptr = std::get_if<Formula>(&e)) {
				const auto& form = *form_ptr;
				os << "φ" << "(" << form << ")";
			}

			if (i != trace.size() - 1) {
				os << ",";
			}
		}
	}

	inline std::ostream& operator<< (std::ostream& os, const Execution& exec) {
		PrintTrace(os, exec.trace);
		if (exec.RecursiveExecutions()) {
			os << "\n";
			os << "[Subexecutions]\n";
			for (const auto& sub : exec.sub_executions) {
				os << "	"; // There could be more indentations (sub-executions of sub-executions) but not practically.
				os << sub;
			}
		}
		
		return os;
	}

	inline std::ostream& operator<< (std::ostream& os, const std::vector<Execution>& exec_vec) {
		os << "[Execution Traces] \n";
		for (const auto& exec : exec_vec) {
			os << exec; 
			os << "\n";
		}
		return os;
	}


}

// @Cleanup: make Trace its own datatype