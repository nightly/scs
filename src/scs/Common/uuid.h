#pragma once

#include <cstdint>
#include <functional>

namespace scs {

	class UUID {
	private:
		uint64_t id_;
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		uint64_t Get() const {
			return id_;
		}

		bool operator==(const UUID& other) const {
			return id_ == other.id_;
		}
	};

}


namespace std {

	template<>
	struct hash<scs::UUID> {
		size_t operator()(const scs::UUID& uuid) const {
			return uuid.Get();
		}
	};

}