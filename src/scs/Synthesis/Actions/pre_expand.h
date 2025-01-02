#pragma once

#include "scs/Synthesis/topology/complete/complete.h"
#include "scs/Synthesis/actions/cache.h"

namespace scs {

	// Optionally pre-expands everything it can (for ungrounded actions -> grounded actions)
	// Requires a complete topology
	Cache PreExpand(const CompleteTopology& top, const ankerl::unordered_dense::set<Object>& objects);

}