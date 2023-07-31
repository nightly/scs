#pragma once

#include <string>
#include <vector>
#include <ostream>

#include "scs/FirstOrderLogic/object.h"
#include <boost/container_hash/hash.hpp>

#include <ankerl/unordered_dense.h>

namespace scs {

	struct RelationalFluent {
	public:
		size_t arity_ = 8080;
	private:
		ankerl::unordered_dense::map<std::vector<Object>, bool, boost::hash<std::vector<Object>>> valuations_;
	public:
		RelationalFluent() = default;
		RelationalFluent(size_t arity);

		size_t Arity() const;
		const ankerl::unordered_dense::map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& valuations() const;
		ankerl::unordered_dense::map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& valuations();

		void AddValuation(const std::vector<Object>& params, bool b);
		void AddValuation(std::vector<Object>&& params, bool b);
		void AddValuation(bool b);

		bool Valuation(const std::vector<scs::Object>& objects) const;
		bool Valuation() const;

		std::string ToString() const;
		friend std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent);
		bool operator==(const RelationalFluent& other) const;
		bool operator!=(const RelationalFluent& other) const;

	};

}