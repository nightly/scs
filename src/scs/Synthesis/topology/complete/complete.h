#pragma once

#include <vector>

#include "scs/Synthesis/Topology/interface_topology.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

#include <boost/container_hash/hash.hpp>

namespace scs {

	struct CompleteTopology : public ITopology {
	private:
		nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>> topology_;
		const std::vector<CharacteristicGraph>& graphs_;
	public:
		CompleteTopology(const std::vector<CharacteristicGraph>& graphs, bool recursive = false) : graphs_(graphs) {
			TopologyState initial_key;
			initial_key.reserve(graphs_.size());
			for (const auto& graph : graphs_) {
				initial_key.emplace_back(graph.lts.initial_state());
			}
			topology_.set_initial_state(initial_key);
			if (recursive) {
				CombineRecursive(initial_key);
			} else {
				CombineIterative(initial_key);
			}
		}

		const TopologyState& initial_state() const override {
			return topology_.initial_state();
		}
		
		nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& lts() override {
			return topology_;
		}

		const nightly::State<TopologyState, TopologyTransition>& at(const TopologyState& key) const override {
			return topology_.at(key);
		}

	private:
		void CombineRecursive(const TopologyState& key) {
			for (size_t i = 0; i < graphs_.size(); ++i) {

			}


		}

		void CombineIterative(const TopologyState& key) {

		}
	};
}