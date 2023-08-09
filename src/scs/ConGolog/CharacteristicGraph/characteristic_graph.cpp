#include "characteristic_graph.h"

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/Combinatorics/Utils/contains.h"

namespace scs {

	CharacteristicGraph::CharacteristicGraph() {
		lts.set_initial_state(0);
	}

	CharacteristicGraph::CharacteristicGraph(std::shared_ptr<IProgram> ptr, ProgramType type, bool print_info) : type(type) {
		lts.set_initial_state(0);
		StateCounter counter;
		StateTracker tracker;
		ptr->AddTransition(*this, counter, tracker);
		MarkStates();

		if (print_info) {
			std::cout << tracker << std::endl;
			std::cout << counter << std::endl;
		}
	}

	void CharacteristicGraph::MarkStates() {
		// Assumptive on resource/recipe modelling (e.g. resource \delta^*). Should be fine simplification.
		if (type == ProgramType::Resource) {
			auto state = lts.at(0);
			lts.EraseShallow(0);
			lts.AddState(CgState(0, true), state);
			lts.set_initial_state(CgState(0, true));

			// Mark any transitions at any other state that go back to state 0, change the final cond to true
			for (auto& [state, internal] : lts.states()) {
				for (auto& trans : internal.transitions()) {
					if (trans.to().n == 0) {
						trans.to().final_condition = Formula(true);
					}
				}
			}
		} else if (type == ProgramType::Recipe) {
			std::vector<CgState> state_changes;
			std::vector<nightly::State<scs::CgState, scs::CgTransition>> transition_changes;
			// Find final states (those that do not have transitions)
			for (auto& [state, internal] : lts.states()) {
				if (internal.transitions().empty()) {
					state_changes.emplace_back(state);
					transition_changes.emplace_back(internal);
				}
			}
			// Erase and add the new
			for (size_t i = 0; i < state_changes.size(); ++i) {
				state_changes[i].final_condition = true;
				lts.EraseShallow(state_changes[i].n);
				lts.AddState(state_changes[i], transition_changes[i]);
			}

			// Rewrite transitions that mention the old final states
			for (auto& [state, internal] : lts.states()) {
				for (auto& trans : internal.transitions()) {
					if (Contains(state_changes, trans.to())) {
						trans.to().final_condition = true;
					}
				}
			}
		}

	}

	std::ostream& operator<< (std::ostream& os, const CharacteristicGraph& graph) {
		os << graph.lts;
		return os;
	}
}