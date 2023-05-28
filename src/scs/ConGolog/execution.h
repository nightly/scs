#pragma once

#include <vector>
#include <variant>
#include <ostream>

#include "scs/ConGolog/trace.h"
#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/fol.h"

namespace scs {
	
	struct Execution {
	public:
		Trace trace;
		std::vector<Execution> sub_executions; // Note: sub_executions can only be traced at the end of trace. And sub execution path must be followed at least (if exists).
	public:
		bool SubExecutions() const { return !sub_executions.empty(); }
		friend std::ostream& operator<< (std::ostream& os, const Execution& exec);
	};

	inline void PrintExecution(std::ostream& os, const Execution& exec, size_t indent = 0) {
		Trace::PrintTrace(os, exec.trace, indent);
		indent++;
		if (exec.SubExecutions()) {
			os << "\n";
			// os << "[Subexecutions]\n";
			for (const auto& sub : exec.sub_executions) {
				PrintExecution(os, sub, indent);
				os << "\n";
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
		size_t count = 1;
		for (const auto& exec : exec_vec) {
			os << "[Execution Trace " << count << " ]" << "\n";
			count++;
			os << exec; 
			os << "\n";
		}
		return os;
	}

}

// @Cleanup: make Trace its own datatype, especially because the non_det_iterative is being tracked manually