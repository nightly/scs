#pragma once

#include <vector>
#include <span>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

#include <boost/container_hash/hash.hpp>
#include "lts/lts.h"

namespace scs {

	using TopologyState = std::vector<CgState>;
	using TopologyTransition = std::vector<CgTransition>;

	class ITopology {
		virtual ~ITopology() = default;

		virtual const TopologyState& initial_state() const = 0;
		virtual nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& lts() = 0;

		virtual const nightly::State<TopologyState, TopologyTransition>& at(const TopologyState& key) const = 0;
	};

}