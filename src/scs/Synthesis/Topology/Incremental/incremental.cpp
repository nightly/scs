#include "incremental.h"

namespace scs {

	IncrementalTopology::IncrementalTopology(const std::vector<CharacteristicGraph>* graphs) : graphs_(graphs) {
		TopologyState initial_key;
		initial_key.reserve(graphs_->size());
		for (const auto& graph : *graphs_) {
			initial_key.emplace_back(graph.lts.initial_state());
		}
		topology_.set_initial_state(initial_key);
		ExpandState(initial_key);
	}

	const TopologyState& IncrementalTopology::initial_state() const {
		return topology_.initial_state();
	}

	const nightly::LTS<TopologyState, TopologyTransition, boost::hash<TopologyState>>& IncrementalTopology::lts() const {
		return topology_;
	}

	const nightly::State<TopologyState, TopologyTransition>& IncrementalTopology::at(const TopologyState& key) {
		if (!visited_.contains(key)) {
			ExpandState(key);
		}
		return topology_.at(key);
	}

	void IncrementalTopology::ExpandState(const TopologyState& key) {
		visited_.emplace(key);

		std::vector<const std::vector<nightly::Transition<scs::CgState, scs::CgTransition>>*> vecs;
		for (size_t i = 0; i < graphs_->size(); ++i) {
			vecs.emplace_back(&graphs_->at(i).lts.at(key[i]).transitions());
		}

		auto product = Product(vecs, FlagValue());
		for (const auto& prod : product) {
			auto transition = CreateTransition(prod);
			topology_.AddTransition(key, transition.label(), transition.to());
		}
	}
}