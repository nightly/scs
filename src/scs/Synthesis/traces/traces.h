#pragma once

#include <unordered_set>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/actions/instantiations.h"
#include "scs/Synthesis/traces/evolution.h"

#include "boost/container_hash/hash.hpp"

namespace scs {
	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct Traces {
	public:
		const std::unordered_set<Object>* objects_;
		ActionInstantiations inst_;

		std::unordered_map<std::vector<TransitionType>, std::vector<Evolution>, 
			boost::hash<std::vector<TransitionType>>> cache;
	public:
		Traces(const std::unordered_set<Object>& objects);

		const std::vector<Evolution>& Get(const std::vector<TransitionType>& transitions);
		
		void Expand(const std::vector<TransitionType>& transitions);
	};
}