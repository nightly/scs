#pragma once

#include <span>
#include <string>
#include <vector>

#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/coop_matrix.h"
#include "scs/SituationCalculus/routes_matrix.h"
#include "scs/FirstOrderLogic/operators.h"
#include "scs/SituationCalculus/action_type.h"
#include "scs/SituationCalculus/poss_union.h"
#include "scs/SituationCalculus/poss_union_default.h"

#include "ankerl/unordered_dense.h"

namespace scs {

	struct BasicActionTheory {
	public:
		ankerl::unordered_dense::map<std::string, Poss> pre;
		ankerl::unordered_dense::map<std::string, Successor> successors;
		ankerl::unordered_dense::map<std::string, ActionType> types;

		ankerl::unordered_dense::set<Object> objects;

		PossUnion poss_mappings;
		bool is_global;
	private:
		Situation initial_; // Encapsulates initial situation description
		CoopMatrix coop_mx_;
		RoutesMatrix routes_mx_;
	public:
		const Situation& Initial() const {
			return initial_;
		}

		const CoopMatrix& CoopMx() const {
			return coop_mx_;
		}

		const RoutesMatrix& RoutesMx() const {
			return routes_mx_;
		}

		template <typename S>
		void SetInitial(S&& initial_situation) {
			assert(initial_situation.history.size() == 0 && "Initial situation cannot have a history of actions");
			initial_ = std::forward<S>(initial_situation);
		}

		template <typename M>
		void SetCoopMx(M&& CoopMx) {
			assert(is_global && "Coop Matrix being set for a non-global BAT");
			coop_mx_ = std::forward<M>(CoopMx);
		}

		template <typename M>
		void SetRoutesMx(M&& RoutesMx) {
			assert(is_global && "Routes Matrix being set for a non-global BAT");
			routes_mx_ = std::forward<M>(RoutesMx);
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

		os << "CoopMatrix: " << bat.CoopMx() << "\n";

		os << "RoutesMatrix: " << bat.RoutesMx() << "\n";

		os << bat.Initial();

		return os;
	}
}