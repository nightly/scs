#pragma once

#include "scs/Synthesis/topology/complete/complete.h"
#include "scs/Synthesis/actions/cache.h"

namespace scs {

	// Optionally pre-expands everything it can, needs a complete topology
	CompoundActionCache PreExpand(const CompleteTopology& top, const ankerl::unordered_dense::set<Object>& objects) {
		CompoundActionCache ret(objects);
		for (const auto& [state, internal] : top.lts().states()) {
			for (const auto& trans : internal.transitions()) {
				ret.Get(trans.label().act);
			}
		}
		return ret;
	}

}