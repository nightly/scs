#pragma once

#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"

#include "boost/container_hash/hash.hpp"

namespace scs {

	struct Permutation {
		size_t r;
		std::unordered_set<Object> used;

		bool operator==(const Permutation& other) const {
			return (r == other.r && used == other.used);
		}

		bool operator!=(const Permutation& other) const {
			return !(*this == other);
		}
	};

}

namespace std {
	template <>
	struct std::hash<scs::Permutation> {
		size_t operator() (const scs::Permutation& perm) const {
			size_t seed = 0;
			boost::hash<size_t> hasher;
			boost::hash_combine(seed, hasher(perm.r));
			boost::hash_unordered_range(seed, perm.used.begin(), perm.used.end());
			return seed;
		}
	};

}