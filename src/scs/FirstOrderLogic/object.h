#pragma once

#include <string>
#include <vector>

#include <boost/container_hash/hash.hpp>

namespace scs {

	class Situation;

	class Object {
	private:
		std::string name_;
	public:
		Object() = default;
		Object(const std::string& name) : name_(name) {}
		Object(std::string&& name) : name_(std::move(name)) {}
		Object(const char* name) : name_(name) {}
		Object(const std::string& name, Situation& s) : name_(name) {
			AddObjectToDomain(s);
		}
		Object(std::string&& name, Situation& s) : name_(std::move(name)) {
			AddObjectToDomain(s);
		}
		Object(const char* name, Situation& s) : name_(name) {
			AddObjectToDomain(s);
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
		void AddObjectToDomain(Situation& s);
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

	void ObjectUSetPrint(const std::unordered_set<Object>& set, std::ostream& os = std::cout, std::string_view delimiter = ", ", std::string_view indent = "");

	inline std::size_t hash_value(const Object& o) {
		boost::hash<std::string> hasher;
		return hasher(o.name());
	}

}

namespace std {
	template <>
	struct std::hash<scs::Object> {
		size_t operator() (const scs::Object& obj) const {
			return hash<std::string>()(obj.name());
		}
	};

}

// Aliases?
