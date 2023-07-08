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


*/

inline std::shared_ptr<IProgram> ExampleRecipe() {
	scs::ActionProgram Load1{scs::Action{"Load", {Object{"p1"}} }};
	scs::ActionProgram Load2{scs::Action{"Load", {Object{"p2"}} }};

	return std::make_shared<Sequence>(Load1, Load2);
}
