#pragma once

#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/poss.h"
#include "scs/FirstOrderLogic/operators.h"
#include "scs/SituationCalculus/action_type.h"
#include "scs/SituationCalculus/rigid_database.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	struct BasicActionTheory {
	public:
		ankerl::unordered_dense::map<std::string, Poss> pre;
		ankerl::unordered_dense::map<std::string, Successor> successors;
		ankerl::unordered_dense::map<std::string, ActionType> types;

		ankerl::unordered_dense::set<Object> objects;
		ankerl::unordered_dense::set<Object> rigid_objects;
		RigidDatabase rigid;
		RigidDatabase initial_declarations;

	private:
		Situation initial_; // Encapsulates initial situation description
	public:
		const Situation& Initial() const {
			return initial_;
		}

		template <typename S>
		void SetInitial(S&& initial_situation) {
			if (!initial_situation.history.empty()) {
				throw std::invalid_argument("Initial situation cannot have a history of actions");
			}
			initial_ = std::forward<S>(initial_situation);
		}

		void PrintObjects(std::ostream& os, size_t indent) const {
			std::string indent_space(indent, ' ');
			ObjectUSetPrint(this->objects, os, ", ");
			os << "}\n";
		}


	};

	inline std::ostream& operator<< (std::ostream& os, const BasicActionTheory& bat) {
		std::string indent_space(2, ' ');
		os << "Preconditions:\n";
		for (const auto& pre : bat.pre) {
			os << indent_space << pre.first << "(" << TermVectorToString(pre.second.Terms()) << ")";
			os << " = ";
			os << pre.second.Form();
			os << "\n";
		}

		os << "Successors:\n";
		for (const auto& suc : bat.successors) {
			os << indent_space << suc.first << "(" << TermVectorToString(suc.second.Terms()) << ")";
			os << " = ";
			os << suc.second.Form();
			os << "\n";
		}

		os << "Objects (" << bat.objects.size() << "):\n";
		os << indent_space << "{";
		for (auto it = bat.objects.begin(); it != bat.objects.end(); ++it) {
			if (it != bat.objects.begin()) {
				os << ", ";
			}
			os << *it;
		}
		os << "}\n";

		os << bat.Initial();

		return os;
	}
}
