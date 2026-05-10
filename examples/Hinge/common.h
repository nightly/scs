#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

namespace scs::examples {

	struct HingeCommon {
	public:
		Formula Nop_form = ParseScFormula("true");
		Formula within_reach = ParseScFormula("exists j. j != i and at(part, j) and coop(i, j)");

		// Util (naming differences)
		Formula within_reach_part_1 = ParseScFormula("exists j. j != i and at(part1, j) and coop(i, j)");
		Formula within_reach_part_2 = ParseScFormula("exists j. j != i and at(part2, j) and coop(i, j)");
	};

	inline BasicActionTheory HingeCommonBAT() {
		return ParseBasicActionTheory(R"(
objects brass, tube, 5
type Nop = nop
type In = transfer
type Out = transfer

init on_site(brass) = true
init on_site(tube) = true
init part(brass) = true
init part(tube) = true
init material(brass) = true
init suitable(5mm, 1.5) = true
init clamped(brass, 5, 1) = false
init clamped(tube, 5, 1) = false
init clamped(tube, 0.5, 1) = false
init bit(3mm) = true
init bit(5mm) = true
init equipped_bit(3mm, 3) = false
init equipped_bit(5mm, 3) = false
init at(brass, 1) = false
init at(brass, 2) = false
init at(brass, 3) = false
init at(brass, 4) = false
init at(tube, 1) = false
init at(tube, 2) = false
init at(tube, 3) = false
init at(tube, 4) = false
init status(ok) = true
init pigment(metallic_blue) = true
init pigment(metallic_red) = true

poss Nop = true
poss Out(part, i) = part(part) and at(part, i)
poss In(part, i) = part(part) and not (exists p. at(p, i))

ssa at(part, i) = a = In(part, i) or a = Load(part, i) or (cv and a != Out(part, i))
ssa on_site(part) = cv and not (exists i. a = Load(part, i))
)");
	}
}
