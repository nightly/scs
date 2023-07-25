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

	struct CompleteTopology : public ITopology {
	private:
		nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>> topology_;
		const std::vector<CharacteristicGraph>* graphs_;

		std::unordered_set<TopologyState, boost::hash<TopologyState>> visited_;
	public:
		CompleteTopology(const std::vector<CharacteristicGraph>* graphs, bool recursive = false) : graphs_(graphs) {
			TopologyState initial_key;
			initial_key.reserve(graphs_->size());
			for (const auto& graph : *graphs_) {
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

		const nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& lts() const override {
			return topology_;
		}

		const nightly::State<TopologyState, TopologyTransition>& at(const TopologyState& key) override {
			return topology_.at(key);
		}

	private:
		void CombineRecursive(const TopologyState& key) {
			if (visited_.contains(key)) {
				return;
			}
			visited_.insert(key);

			std::vector<const std::vector<nightly::Transition<scs::CgState, scs::CgTransition>>*> vecs;
			for (size_t i = 0; i < graphs_->size(); ++i) {
				vecs.emplace_back(&graphs_->at(i).lts.at(key[i]).transitions());
			}

			auto product = Product(vecs, FlagValue());
			for (const auto& prod : product) {
				auto transition = CreateTransition(prod);
				topology_.AddTransition(key, transition.label(), transition.to());
				CombineRecursive(transition.to());
			}
		}

		void CombineIterative(const TopologyState& key) {
			std::stack<TopologyState> stack;
			stack.push(key);

			while (!stack.empty()) {
				auto current_key = std::move(stack.top());
				stack.pop();
				if (visited_.contains(key)) {
					continue;
				}
				visited_.insert(key);

				std::vector<const std::vector<nightly::Transition<scs::CgState, scs::CgTransition>>*> vecs;
				for (size_t i = 0; i < graphs_->size(); ++i) {
					vecs.emplace_back(&graphs_->at(i).lts.at(current_key[i]).transitions());
				}

				auto product = Product(vecs, FlagValue());
				for (const auto& prod : product) {
					auto transition = CreateTransition(prod);
					topology_.AddTransition(current_key, transition.label(), transition.to());
					stack.push(transition.to());
				}
			}
		}

	};
}