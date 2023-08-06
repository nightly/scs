#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#Program
	Load(brass) || Load(tube);
	Clamp(brass) ||| RadialDrill(brass, 5);
	ApplyAdhesive(tube, brass)
*/

namespace scs::examples {

	inline std::shared_ptr<IProgram> HingeRecipe() {
		scs::ActionProgram LoadBrass{scs::Action{"Load", {Object{"brass"}} }};
		scs::ActionProgram LoadTube{scs::Action{"Load", {Object{"tube"}} }};

		scs::ActionProgram Clamp{scs::Action{"Clamp", { Object{"brass"} }}};
		scs::ActionProgram RadialDrill{scs::Action{"RadialDrill", { Object{"brass"}, Object{"5mm"}}}};

		scs::ActionProgram ApplyAdhesive{scs::Action{"ApplyAdhesive", { Object{"brass"}, Object{"tube"} }}};
		// ------ //

		Interleaved interleaved(LoadBrass, LoadTube);
		Simultaneous sim1(Clamp, RadialDrill);

		Sequence stage1(interleaved, sim1);
		Sequence stage2(stage1, ApplyAdhesive);

		return std::make_shared<Sequence>(stage2);
	}
}