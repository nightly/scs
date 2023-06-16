#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

inline void FluentProg_() {
	scs::SetConsoleEncoding();
	scs::RelationalFluent holding{ "Holding" }, holding_params{ "HoldingP" };

	holding.AddValuation(true);
	holding_params.AddValuation(std::vector<scs::Object>{"robot", "plate"}, false);

	scs::Formula f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot2" }, BinaryKind::NotEqual);
	std::cout << f;
}


// FluentProg_();
// s_allocation_metrics.PrintUsage();