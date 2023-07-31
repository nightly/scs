#include "scs/SituationCalculus/relational_fluent.h"

#include <string>
#include <unordered_set>
#include <ostream>
#include <cassert>

#include "scs/Common/strings.h"
#include "scs/Common/log.h"

namespace scs {

	// == Ctors, dtors ==
	RelationalFluent::RelationalFluent(size_t arity) : arity_(arity) {}

	// == Getters == 

	size_t RelationalFluent::Arity() const {
		return arity_;
	}

	const ankerl::unordered_dense::map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& RelationalFluent::valuations() const {
		return valuations_;
	}

	ankerl::unordered_dense::map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& RelationalFluent::valuations() {
		return valuations_;
	}
	
	// == General ==

	std::string RelationalFluent::ToString() const {
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

	/*
	 * @brief: A valuation for empty parameters
	 */
	void RelationalFluent::AddValuation(bool b) {
		assert((arity_ == 0 || arity_ == 8080) && "Adding valuation to fluent that has previously set different arity");
		arity_ = 0;
		valuations_[{{"0-arity"}}] = b;
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(const std::vector<Object>& params, bool b) {
		assert((params.size() == arity_ || arity_ == 8080) && "Adding valuation to fluent that has previously set different arity");
		arity_ = params.size();
		valuations_[params] = b;
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(std::vector<Object>&& params, bool b) {
		assert((params.size() == arity_ || arity_ == 8080) && "Adding valuation to fluent that has previously set different arity");
		arity_ = params.size();
		valuations_[params] = std::move(b);
	}
	
	bool RelationalFluent::Valuation(const std::vector<scs::Object>& objects) const {
		assert((objects.size() == arity_ || arity_ == 8080) && "Searching valuation in Relational Fluent that has different arity than stored");

		if (!valuations_.contains(objects)) {
			SCS_DEBUG("The valuation of objects {} doesn't exist", ObjectVectorToString(objects));
			return false; // @Assumption: closed world assumption
		} else {
			return valuations_.at(objects);
		}
	}

	// 0-arity evaluation (no parameter fluent)
	bool RelationalFluent::Valuation() const {
		assert((arity_ == 0) && "Looking for 0-arity valuation in Relational Fluent that has non 0-arity stores");
		return valuations_.at({{"0-arity"}});
	}


	// == Operator overloads ==

	std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent) {
		if (fluent.Arity() == 0) {
			stream << "Fluent" << " = " << BoolToString(fluent.valuations_.at({"0-arity"}));
			return stream;
		}

		for (auto it = fluent.valuations_.begin(); it != fluent.valuations_.end(); ++it) {
			if (it != fluent.valuations_.begin()) {
				stream << ", ";
			}
			const auto& val = *it;
			stream << "(" << ObjectVectorToString(val.first) << ")" << " = " << BoolToString(val.second);
		}

		return stream;
	}

	bool RelationalFluent::operator==(const RelationalFluent& other) const {
		return (valuations_ == other.valuations_);
	}

	bool RelationalFluent::operator!=(const RelationalFluent& other) const {
		return (*this != other);
	}

}