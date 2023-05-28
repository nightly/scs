#pragma once

#include <vector>
#include <variant>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/fol.h"

namespace scs {
	
	struct Trace {
	public:
		std::vector<std::variant<Action, Formula>> trace;
		bool non_det_iterative = false;

		bool Iteration() const {
			return non_det_iterative;
		}

	public:
		void Add(const Action& act) {
			trace.emplace_back(act);
		}

		void Add(const Formula& form) {
			trace.emplace_back(form);
		}

		void Add(Action&& act) {
			trace.emplace_back(std::move(act));
		}

		void Add(Formula&& form) {
			trace.emplace_back(std::move(form));
		}

	public:
		// @brief: Print Trace
		static void PrintTrace(std::ostream& os, const Trace& trace, size_t indent = 0) {
			std::string indent_space(indent, ' ');
			if (trace.trace.empty()) {
				os << indent_space << "Empty initial trace";
				return;
			}

			if (trace.non_det_iterative) {
				os << indent_space << "(";
			} else {
				os << indent_space;
			}
			for (size_t i = 0; i < trace.trace.size(); ++i) {
				const auto& e = trace.trace[i];

				if (auto act_ptr = std::get_if<Action>(&e)) {
					const auto& act = *act_ptr;
					os << act;
				} else if (auto form_ptr = std::get_if<Formula>(&e)) {
					const auto& form = *form_ptr;
					os << "φ" << "(" << form << ")";
				}

				if (i != trace.trace.size() - 1) {
					os << ",";
				}
			}
			if (trace.non_det_iterative) {
				os << ")*";
			}
		}


	};

}