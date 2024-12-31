#pragma once

#include "ankerl/unordered_dense.h"

#include "scs/SituationCalculus/situation_calculus.h"

namespace scs {

	// Stores the legal actions for a given situation
	struct SituationCache {
	public:
		ankerl::unordered_dense::map<Situation, CompoundAction> cache_;
	public:
		
		void AddAction(const Situation& s, const CompoundAction& ca);

		bool Exists(const Situation& s);

		template <typename ContainerT>
		void AddActions(const Situation& s, const ContainerT& cas) {
			for (auto const& ca : cas) {
				AddAction(s, ca);
			}
		}

	};
}