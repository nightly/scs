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
	ApplyAdhesive(brass, tube); Store(brass, ok);
*/

namespace scs::examples {

	inline std::shared_ptr<IProgram> HingeRecipe(size_t variant = 3) {
		scs::ActionProgram LoadBrass{scs::Action{"Load", {Object{"brass"}} }};
		scs::ActionProgram LoadTube{scs::Action{"Load", {Object{"tube"}} }};

		scs::ActionProgram Clamp{scs::Action{"Clamp", { Object{"brass"} }}};
		scs::ActionProgram RadialDrill{scs::Action{"RadialDrill", { Object{"brass"}, Object{"5mm"}}}};

		scs::ActionProgram ApplyAdhesive{scs::Action{"ApplyAdhesive", { Object{"brass"}, Object{"tube"} }}};
		scs::ActionProgram Store{scs::Action{"Store", { Object{"brass"}, Object{"ok"} }}};
		// ------ //

		Interleaved interleaved(LoadBrass, LoadTube);
		Simultaneous sim1(Clamp, RadialDrill);

		Sequence stage1(interleaved, sim1);
		Sequence stage2(stage1, ApplyAdhesive);
		Sequence stage3(stage2, Store);

		if (variant == 1) {
			return std::make_shared<Sequence>(stage1);
		} else if (variant == 2) {
			return std::make_shared<Sequence>(stage2);
		} else if (variant == 3) {
			return std::make_shared<Sequence>(stage3);
		} else {
		 	return std::make_shared<Sequence>(stage3);
		}
	}
}