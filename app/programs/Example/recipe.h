#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

using namespace scs;

/*
#Program
	Load(brass) || Load(tube);
	Paint(brass, metallic blue) | Paint(brass, metallic red);
	Clamp(brass) ||| RadialDrill(brass, 5);
	ApplyAdhesive(tube, brass)
*/

inline std::shared_ptr<IProgram> ExampleRecipe() {
	scs::ActionProgram LoadBrass{scs::Action{"Load", {Object{"brass"}} }};
	scs::ActionProgram LoadTube{scs::Action{"Load", {Object{"tube"}} }};

	scs::ActionProgram PaintBlue{scs::Action{"Paint", { Object{"brass"}, Object{"metallic red"} }}};
	scs::ActionProgram PaintRed{scs::Action{"Paint", { Object{"brass"}, Object{"metallic blue"} }}};

	scs::ActionProgram Clamp{scs::Action{"Clamp", { Object{"brass"} }}};
	scs::ActionProgram RadialDrill{scs::Action{"RadialDrill", { Object{"brass"}, Object{"0.5"}}}};

	scs::ActionProgram ApplyAdhesive{scs::Action{"ApplyAdhesive", { Object{"tube"}, Object{"brass"} }}};

	// ------ //

	Interleaved interleaved(LoadBrass, LoadTube);
	Branch nd1(PaintBlue, PaintRed);
	Simultaneous sim1(Clamp, RadialDrill);

	Sequence stage1(interleaved, nd1);
	Sequence stage2(stage1, sim1);
	Sequence stage3(stage2, ApplyAdhesive);

	return std::make_shared<Sequence>(stage3);
}
