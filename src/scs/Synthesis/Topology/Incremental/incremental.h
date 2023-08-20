#pragma once

#include <vector>
#include <unordered_set>
#include <stack>

#include "scs/Synthesis/Topology/interface_topology.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Combinatorics/CartesianProduct/product.h"
#include "scs/Synthesis/Topology/core.h"

#include <boost/container_hash/hash.hpp>

namespace scs {

	struct IncrementalTopology : public ITopology {
	private:
		nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>> topology_;
		const std::vector<CharacteristicGraph>* graphs_;

		std::unordered_set<TopologyState, boost::hash<TopologyState>> visited_;
	public:
		IncrementalTopology(const std::vector<CharacteristicGraph>* graphs);

		const TopologyState& initial_state() const override;
		const nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& lts() const override;
		nightly::State<TopologyState, TopologyTransition>& at(const TopologyState& key) override;

	private:

		void ExpandState(const TopologyState& key);

	};
}
