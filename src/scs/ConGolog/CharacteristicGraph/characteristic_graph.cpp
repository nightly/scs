#include "characteristic_graph.h"

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/ConGolog/Program/programs.h"

namespace scs {

	CharacteristicGraph::CharacteristicGraph() {
		lts.set_initial_state(0);
	}

	CharacteristicGraph::CharacteristicGraph(std::shared_ptr<IProgram> ptr, ProgramType type) : type(type) {
		lts.set_initial_state(0);
		StateCounter counter;
		StateTracker tracker;
		StateMeta meta;
		ptr->AddTransition(*this, counter, tracker, meta);
		MarkStates();
	}

	void CharacteristicGraph::MarkStates() {
		// Assumptive on resource/recipe modelling (e.g. resource \delta^*). Should be fine simplification.
		if (type == ProgramType::Resource) {
			auto state = lts.at(0);
			lts.EraseShallow(0);
			lts.AddState(CgState(0, true), state);
		} else if (type == ProgramType::Recipe) {
			std::vector<CgState> state_changes;
			std::vector<nightly::State<scs::CgState, scs::CgTransition>> transition_changes;
			for (auto& pair : lts.states()) {
				if (pair.second.transitions().empty()) {
					state_changes.emplace_back(pair.first);
					transition_changes.emplace_back(pair.second);
				}
			}
			for (size_t i = 0; i < state_changes.size(); ++i) {
				state_changes[i].final_condition = true;
				lts.EraseShallow(state_changes[i].n);
				lts.AddState(state_changes[i], transition_changes[i]);
			}
		}

	}

	std::ostream& operator<< (std::ostream& os, const CharacteristicGraph& graph) {
		os << graph.lts;
		return os;
	}
}