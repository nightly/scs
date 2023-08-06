#pragma once

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#Program
	Load(brass) || Load(tube);
*/

namespace scs::examples {

	inline std::shared_ptr<IProgram> HingeRecipeQuick() {
		scs::ActionProgram LoadBrass{scs::Action{"Load", {Object{"brass"}} }};
		scs::ActionProgram LoadTube{scs::Action{"Load", {Object{"tube"}} }};

		scs::ActionProgram PaintRed{scs::Action{"Paint", { Object{"brass"}, Object{"metallic_blue"} }}};

		// ------ //

		Interleaved interleaved(LoadBrass, LoadTube);
		Sequence stage1(interleaved, PaintRed);
		return std::make_shared<Sequence>(stage1);
	}
}