#include "scs/Common/uuid.h"

#include <random>
#include <unordered_map>
#include <cstdint>

namespace scs {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID() : id_(s_UniformDistribution(s_Engine)) {}

	UUID::UUID(uint64_t uuid) : id_(uuid) {}
}