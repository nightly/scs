#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/poss_union_default.h"

namespace scs {

	struct PossUnion {
	public:
		using ContainmentCheckFunc = bool(*)(const CompoundAction&);
		using ActionCheckFunc = bool(*)(const Situation&, const CompoundAction&, const BasicActionTheory&);

		struct ActionEntry {
			ContainmentCheckFunc contain_check;
			ActionCheckFunc action_check;
		};
	public:
		std::vector<ActionEntry> mappings = { 
			{ ContainsTransfer, PossibleTransfer },
			{ ContainsRadialAndClamp, PossibleRadial } 
		};
	public:
		PossUnion() = default;
		PossUnion(const std::vector<ActionEntry>& actionMappings) : mappings(mappings) {}
	};


}