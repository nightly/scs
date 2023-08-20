#include "scs/Synthesis/Topology/Complete/utils.h"
#include "scs/Synthesis/Topology/Complete/complete.h"

#include <random>

namespace scs {

	// This isn't really useful aside from testing purposes
	void ShuffleComplete(CompleteTopology& ct) {
		std::random_device rd;
		std::mt19937 rng_{ rd() };
		for (auto& [p1, p2] : ct.lts().states()) {
			ct.at(p1).transitions_shuffled(rng_);
		}
	}

}