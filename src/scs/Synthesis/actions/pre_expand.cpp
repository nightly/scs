#include "pre_expand.h"

#include "scs/Synthesis/topology/complete/complete.h"
#include "scs/Synthesis/actions/cache.h"

#include "scs/Common/log.h"

namespace scs {

	// Optionally pre-expands everything it can, needs a complete topology
	CompoundActionCache PreExpand(const CompleteTopology& top, const ankerl::unordered_dense::set<Object>& objects) {
		SCS_INFO("Starting pre-expansion");
		CompoundActionCache ret(objects);
		for (const auto& [state, internal] : top.lts().states()) {
			for (const auto& trans : internal.transitions()) {
				ret.Get(trans.label().act);
			}
		}
		SCS_INFO("Completed pre-expansion");
		return ret;
	}

}