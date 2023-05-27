#pragma once

#include <vector>
#include <variant>
#include <ostream>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/fol.h"

namespace scs {
	
	struct Execution {
	public:
		std::vector<std::variant<Action, Formula>> trace;
		std::vector<Execution> sub_executions; // Note: sub_executions can only be traced at the end of trace. And sub execution path must be followed at least (if exists).
	
		bool non_det_iterative_ = false;
	public:
		bool RecursiveExecutions() const { return !sub_executions.empty(); }
		friend std::ostream& operator<< (std::ostream& os, const Execution& exec);
	};

	// @brief: Print Trace
	inline void PrintTrace(std::ostream& os, const std::vector<std::variant<Action, Formula>>& trace, bool non_det_iterative = false, size_t indent = 0) {
		std::string indent_space(indent, ' ');
		os << indent_space;
		if (non_det_iterative) {
			os << "(";
		}
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
		if (non_det_iterative) {
			os << ")*";
		}
	}

	inline void PrintExecution(std::ostream& os, const Execution& exec, size_t indent = 0) {
		PrintTrace(os, exec.trace, indent);
		if (exec.RecursiveExecutions()) {
			os << "\n";
			os << "[Subexecutions]\n";
			for (const auto& sub : exec.sub_executions) {
				PrintExecution(os, sub, indent + 1);
			}
		}
	}

	// @brief: << Execution
	inline std::ostream& operator<< (std::ostream& os, const Execution& exec) {
		PrintExecution(os, exec);
		return os;
	}



	// ====================================================================
	// Utilities (stuff defined for containers etc)
	// ====================================================================


	// @brief: Print Execution vector
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