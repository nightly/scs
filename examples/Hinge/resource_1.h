#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#BAT

#Program
loop:
	Nop* | In(part, 1); (Nop | (Clamp(part, force, 1); Release(part, 1))*; (Out(part, 1) | Store(part, code, 1))
*/

namespace scs::examples {

	inline Resource HingeResource1() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Nop{ "Nop", {} };
		scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
		scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
		scs::Action Release{ "Release", { Variable{"part"}, Object{"1"} }};
		scs::Action Store{ "Store", { Variable{"part"}, Variable{"code"}, Object{"1"} } };

		Branch nd1(ActionProgram{ Nop }, Sequence(ActionProgram{ Clamp }, ActionProgram{ Release }));
		Loop l1(nd1);
		Sequence s1(ActionProgram{In}, l1);
		Sequence s2(s1, Branch(ActionProgram{Out}, ActionProgram{Store}));

		Branch nd2(ActionProgram{ Nop }, s2);

		ret.bat = ParseBasicActionTheory(R"(
objects 1, ok
type Clamp = preparatory
type Release = preparatory
type Store = manufacturing

init safe_force(brass, 5) = true
init safe_force(tube, 5) = true
init safe_force(tube, 0.5) = true

poss Clamp(part, force, i) = part(part) and at(part, i) and safe_force(part, force) and not (exists f. clamped(part, f, i))
poss Release(part, i) = part(part) and at(part, i) and exists f. clamped(part, f, i)
poss Store(part, code, i) = at(part, i) and status(code)

ssa clamped(part, force, i) = a = Clamp(part, force, i) or (cv and not (a = Release(part, i)))
ssa part(part) = cv and not (exists i, code. a = Store(part, code, i)) and not (exists other, i. a = ApplyAdhesive(other, part, i))
)");

		ret.program = std::make_shared<Loop>(nd2);
		return ret;
	}

}
