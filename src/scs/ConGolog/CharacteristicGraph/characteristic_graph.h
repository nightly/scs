#pragma once

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/LTS/src/lts/lts.h"

namespace scs {

	enum class ProgramType {
		Resource,
		Recipe
	};

	class IProgram;

	class CharacteristicGraph {
	public:
		nightly::LTS<CgState, CgTransition> lts;
		ProgramType type;
	public:
		CharacteristicGraph();

		CharacteristicGraph(std::shared_ptr<IProgram> program_ptr, ProgramType type, bool print_info = false);
	private:
		void MarkStates();
	};

	std::ostream& operator<< (std::ostream& os, const CharacteristicGraph& graph);

}

#include "scs/ConGolog/CharacteristicGraph/export.h"