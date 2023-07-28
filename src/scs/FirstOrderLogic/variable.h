#pragma once

#include <string>

namespace scs {

	class Variable {
	private:
		std::string name_;
	public:
		Variable(const std::string& name) : name_(name) {}
		Variable(const char* name) : name_(name) {}

		const std::string& name() const { return name_; }

		/* Operators */

		operator const std::string& () const {
			return name_;
		}

		bool operator==(const Variable& other) const {
			return (name_ == other.name_);
		}

		bool operator!=(const Variable& other) const {
			return (name_ != other.name_);
		}
	};

	inline std::ostream& operator<< (std::ostream& stream, const Variable& var) {
		stream << var.name();
		return stream;
	}
}

namespace std {
	template <>
	struct std::hash<scs::Variable> {
		size_t operator() (const scs::Variable& var) const {
			return hash<std::string>()("v" + var.name());
		}
	};

}

// Aliases?