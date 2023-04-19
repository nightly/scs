#pragma once

#include <string>
#include <vector>
#include <ostream>

#include "scs/FirstOrderLogic/object.h"
#include <boost/container_hash/hash.hpp>


namespace scs {

	struct RelationalFluent {
	private:
		std::unordered_set<Object> objects_;
		std::string name_;
		std::unordered_map<std::vector<Object>, bool, boost::hash<std::vector<Object>>> valuations_;
	public:
		RelationalFluent() = default;
		RelationalFluent(const std::string& name);
		RelationalFluent(const char* name);
		RelationalFluent(std::string&& name);

		const std::string& name() const;
		const std::unordered_map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& valuations() const;

		void AddValuation(const std::vector<Object>& params, bool b);
		void AddValuation(std::vector<Object>&& params, bool b);
		void AddValuation(bool b);

		bool Valuation(const std::vector<scs::Object>& objects) const;
		bool Valuation() const;

		bool Is0Arity() const;
		bool ContainsObject(const Object& o) const;

		std::string ToString() const;
		friend std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent);
		bool operator==(const RelationalFluent& other) const;
		bool operator!=(const RelationalFluent& other) const;

	};

}

namespace std {

	template <>
	struct std::hash<scs::RelationalFluent> {
		size_t operator() (const scs::RelationalFluent& fluent) const {
			return hash<std::string>()(fluent.name());
		}
	};

}

// Valuation should be its own class?