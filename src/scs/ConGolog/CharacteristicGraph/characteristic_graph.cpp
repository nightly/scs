#include "characteristic_graph.h"

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/ConGolog/Program/programs.h"

namespace scs {

	CharacteristicGraph::CharacteristicGraph() {
		lts.set_initial_state(0);
	}

	CharacteristicGraph::CharacteristicGraph(std::shared_ptr<IProgram> ptr) {
		lts.set_initial_state(0);
		StateCounter counter;
		StateTracker tracker;
		ptr->AddTransition(*this, counter, tracker);
	}

	std::ostream& operator<< (std::ostream& os, const CharacteristicGraph& graph) {
		os << graph.lts;
		return os;
	}
}