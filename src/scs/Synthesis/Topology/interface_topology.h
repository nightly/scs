#pragma once

#include <vector>
#include <span>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "types.h"
#include "out.h"
#include "parse.h"

#include <boost/container_hash/hash.hpp>
#include "lts/lts.h"

namespace scs {

	class ITopology {
	public:
		virtual ~ITopology() = default;

		virtual const TopologyState& initial_state() const = 0;
		virtual const nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& lts() const = 0;

		virtual const nightly::State<TopologyState, TopologyTransition>& at(const TopologyState& key) = 0;
	};

}