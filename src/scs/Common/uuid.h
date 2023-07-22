#pragma once

#include <cstdint>

namespace scs {

	class UUID {
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		uint64_t Get() const {
			return id_;
		}
	private:
		uint64_t id_;
	};

}


namespace std {

	template<>
	struct std::hash<scs::UUID> {
		size_t operator()(const scs::UUID& uuid) const
		{
			return uuid.Get();
		}
	};

}