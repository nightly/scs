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
Arrive(obj(cargo1)); move(obj(cargo1), obj(shipyard)); ship(obj(cargo1))
*/

inline std::shared_ptr<IProgram> ShipRecipe() {
	Action act1{ "Arrive", {scs::Object{"cargo1"}}};
	ActionProgram a1{{act1}};
	Action act2{ "Move", {scs::Object{"cargo1"}, scs::Object{"shipyard"}}};
	ActionProgram a2{{act2}};
	Action act3{ "Ship", {scs::Object{"cargo1"}}};
	ActionProgram a3{{act3}};

	Sequence s1{ a1, a2 };
	Sequence prog{ s1, a3 };

	return std::make_shared<Sequence>(prog);

}