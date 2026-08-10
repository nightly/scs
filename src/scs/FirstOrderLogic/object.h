#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <iostream>

#include <boost/container_hash/hash.hpp>
#include "ankerl/unordered_dense.h"

namespace scs {

	class Situation;
	class BasicActionTheory;

	enum class ObjectKind : unsigned char {
		Identifier,
		Rigid,
	};

	class Object {
	private:
		std::string name_;
		ObjectKind kind_ = ObjectKind::Rigid;
	public:
		Object() = default;
		Object(const std::string& name, ObjectKind kind = ObjectKind::Rigid) : name_(name), kind_(kind) {}
		Object(std::string&& name, ObjectKind kind = ObjectKind::Rigid) : name_(std::move(name)), kind_(kind) {}
		Object(const char* name, ObjectKind kind = ObjectKind::Rigid) : name_(name), kind_(kind) {}
		Object(const std::string& name, BasicActionTheory& bat) : name_(name), kind_(ObjectKind::Rigid) {
			AddObjectToDomain(bat);
		}
		Object(std::string&& name, BasicActionTheory& bat) : name_(std::move(name)), kind_(ObjectKind::Rigid) {
			AddObjectToDomain(bat);
		}
		Object(const char* name, BasicActionTheory& bat) : name_(name), kind_(ObjectKind::Rigid) {
			AddObjectToDomain(bat);
		}

		const std::string& name() const { return name_; }
		ObjectKind kind() const { return kind_; }
		bool IsIdentifier() const { return kind_ == ObjectKind::Identifier; }
		bool IsRigid() const { return kind_ == ObjectKind::Rigid; }

		static Object Identifier(std::string name) {
			return Object{std::move(name), ObjectKind::Identifier};
		}

		static Object Rigid(std::string name) {
			return Object{std::move(name), ObjectKind::Rigid};
		}

		/* Operators */

		operator const std::string&() const { 
			return name_; 
		}

		bool operator==(const Object& other) const {
			return name_ == other.name_ && kind_ == other.kind_;
		}

		bool operator!=(const Object& other) const {
			return !(*this == other);
		}
	private:
		void AddObjectToDomain(BasicActionTheory& bat);
	};

	inline std::ostream& operator<< (std::ostream& stream, const Object& obj) {
		stream << obj.name();
		return stream;
	}

	/* Utility */
	inline std::string ObjectVectorToString(const std::vector<scs::Object>& vec) {
		std::string ret;
		for (size_t i = 0; i < vec.size(); i++) {
			ret += vec[i].name();
			if (i != vec.size() - 1) {
				ret += ", ";
			}
		}
		return ret;
	}

	void ObjectUSetPrint(const ankerl::unordered_dense::set<Object>& set,
		std::ostream& os = std::cout, std::string_view delimiter = ", ", std::string_view indent = "");

	inline std::size_t hash_value(const Object& o) {
		size_t seed = 0;
		boost::hash_combine(seed, o.name());
		boost::hash_combine(seed, static_cast<unsigned char>(o.kind()));
		return seed;
	}

}

namespace std {
	template <>
	struct hash<scs::Object> {
		size_t operator() (const scs::Object& obj) const {
			return scs::hash_value(obj);
		}
	};

}

// Aliases?
