#include "characteristic_graph.h"

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/ConGolog/CharacteristicGraph/compiler.h"
#include "scs/ConGolog/Program/interface_program.h"

namespace scs {

	CharacteristicGraph::CharacteristicGraph() {
		lts.set_initial_state(0);
	}

	CharacteristicGraph::CharacteristicGraph(std::shared_ptr<IProgram> ptr, ProgramType type, bool print_info) : type(type) {
		CompileCharacteristicGraph(*ptr, *this);
		(void)print_info;
	}

	std::ostream& operator<< (std::ostream& os, const CharacteristicGraph& graph) {
		os << graph.lts;
		return os;
	}
}
