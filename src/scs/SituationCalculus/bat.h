#pragma once

#include <span>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/coop_matrix.h"
#include "scs/SituationCalculus/routes_matrix.h"

namespace scs {

	struct BasicActionTheory {
	public:
		std::unordered_map<std::string, Poss> pre;
		std::unordered_map<std::string, Successor> successors;

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

	};

}