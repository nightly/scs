#include "complete.h"

#include <algorithm>

namespace scs {

	CompleteTopology::CompleteTopology(const std::vector<CharacteristicGraph>* graphs, bool recursive) : graphs_(graphs) {
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

	const TopologyState& CompleteTopology::initial_state() const {
		return topology_.initial_state();
	}

	const nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& CompleteTopology::lts() const {
		return topology_;
	}

	nightly::State<TopologyState, TopologyTransition>& CompleteTopology::at(const TopologyState& key) {
		return topology_.at(key);
	}

	void CompleteTopology::CombineRecursive(const TopologyState& key) {
		if (visited_.contains(key)) {
			return;
		}
		visited_.insert(key);

		std::vector<const std::vector<nightly::Transition<scs::CgState, scs::CgTransition>>*> vecs;
		for (size_t i = 0; i < graphs_->size(); ++i) {
			vecs.emplace_back(&graphs_->at(i).lts.at(key[i]).transitions());
		}
		if (std::ranges::any_of(vecs, [](const auto* transitions) { return transitions->empty(); })) {
			return;
		}

		auto product = Product(vecs);
		for (const auto& prod : product) {
			auto transition = CreateTransition(prod);
			topology_.AddTransition(key, transition.label(), transition.to());
			CombineRecursive(transition.to());
		}
	}

	void CompleteTopology::CombineIterative(const TopologyState& key) {
		std::stack<TopologyState> stack;
		stack.push(key);

		while (!stack.empty()) {
			auto current_key = std::move(stack.top());
			stack.pop();
			if (visited_.contains(current_key)) {
				continue;
			}
			visited_.insert(current_key);

			std::vector<const std::vector<nightly::Transition<scs::CgState, scs::CgTransition>>*> vecs;
			for (size_t i = 0; i < graphs_->size(); ++i) {
				vecs.emplace_back(&graphs_->at(i).lts.at(current_key[i]).transitions());
			}
			if (std::ranges::any_of(vecs, [](const auto* transitions) { return transitions->empty(); })) {
				continue;
			}

			auto product = Product(vecs);
			for (const auto& prod : product) {
				auto transition = CreateTransition(prod);
				topology_.AddTransition(current_key, transition.label(), transition.to());
				stack.push(transition.to());
			}
		}
	}

}
