#pragma once

#include <memory>

namespace scs {

	class CharacteristicGraph;
	class IProgram;

	void CompileCharacteristicGraph(const IProgram& program, CharacteristicGraph& graph);

}
