#pragma once

#include <string>
#include <vector>
#include <ostream>

#include "scs/FirstOrderLogic/object.h"

#include <ankerl/unordered_dense.h>
#include <boost/container_hash/hash.hpp>
#include <boost/functional/hash.hpp>

namespace scs {

	struct RelationalFluent {
	public:
		using Tuple = std::vector<Object>;
		using TupleSet = ankerl::unordered_dense::set<Tuple, boost::hash<Tuple>>;

		size_t arity_ = 8080;
	private:
		TupleSet true_tuples_;
	public:
		RelationalFluent() = default;
		RelationalFluent(size_t arity);

		size_t Arity() const;
		const TupleSet& TrueTuples() const;

		void AddValuation(const std::vector<Object>& params, bool b);
		void AddValuation(std::vector<Object>&& params, bool b);
		void AddValuation(bool b);
		void Clear();

		bool Valuation(const std::vector<scs::Object>& objects) const;
		bool Valuation() const;

		std::string ToString() const;
		friend std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent);
		bool operator==(const RelationalFluent& other) const;
		bool operator!=(const RelationalFluent& other) const;

	};

	struct RelationalFluentHash {
		size_t operator()(const RelationalFluent& fluent) const;
	};

}

namespace std {

	template<>
	struct hash<scs::RelationalFluent> {
		size_t operator()(const scs::RelationalFluent& rf) const {
			return scs::RelationalFluentHash{}(rf);
		}
	};
}
