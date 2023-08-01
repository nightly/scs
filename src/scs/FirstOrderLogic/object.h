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

	class Object {
	private:
		std::string name_;
	public:
		Object() = default;
		Object(const std::string& name) : name_(name) {}
		Object(std::string&& name) : name_(std::move(name)) {}
		Object(const char* name) : name_(name) {}
		Object(const std::string& name, BasicActionTheory& bat) : name_(name) {
			AddObjectToDomain(bat);
		}
		Object(std::string&& name, BasicActionTheory& bat) : name_(std::move(name)) {
			AddObjectToDomain(bat);
		}
		Object(const char* name, BasicActionTheory& bat) : name_(name) {
			AddObjectToDomain(bat);
		}

		const std::string& name() const { return name_; }

		/* Operators */

		operator const std::string&() const { 
			return name_; 
		}

		bool operator==(const Object& other) const {
			return (name_ == other.name_);
		}

		bool operator!=(const Object& other) const {
			return (name_ != other.name_);
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
				ret += ',';
			}
		}
		return ret;
	}

	void ObjectUSetPrint(const ankerl::unordered_dense::set<Object>& set,
		std::ostream& os = std::cout, std::string_view delimiter = ", ", std::string_view indent = "");

	inline std::size_t hash_value(const Object& o) {
		boost::hash<std::string> hasher;
		return hasher(o.name());
	}

}

namespace std {
	template <>
	struct std::hash<scs::Object> {
		size_t operator() (const scs::Object& obj) const {
			return hash<std::string>()("o" + obj.name());
		}
	};

}

// Aliases?
