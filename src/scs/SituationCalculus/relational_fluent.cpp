#include "scs/SituationCalculus/relational_fluent.h"

#include <string>
#include <unordered_set>
#include <ostream>
#include <cassert>

#include "scs/Common/strings.h"

namespace scs {

	// == Ctors, dtors ==

	RelationalFluent::RelationalFluent(const std::string& name) : name_(name) {}
	RelationalFluent::RelationalFluent(const char* name) : name_(name) {}
	RelationalFluent::RelationalFluent(std::string&& name) : name_(std::move(name)) {}
	
	// == Getters == 
	const std::string& RelationalFluent::name() const {
		return name_;
	}

	const std::unordered_map<std::vector<Object>, bool, boost::hash<std::vector<Object>>>& RelationalFluent::valuations() const {
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
		// this should have assert but not that important. should track first insert and then base arity on that instead.
		// could also just be tracked by parser.
		valuations_[{{"0-arity"}}] = b;
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(const std::vector<Object>& params, bool b) {
		assert(!Is0Arity() && "Sc: Relational Fluent is inconsistent with parameters having none and some");

		for (const auto& o : params) {
			objects_.emplace(o);
		}
		valuations_[params] = b;
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(std::vector<Object>&& params, bool b) {
		assert(!Is0Arity() && "Sc: Relational Fluent is inconsistent with parameters having none and some");

		for (const auto& o : params) {
			objects_.emplace(o);
		}

		if (valuations_.contains(params)) {
			valuations_[params] = b;
		} else {
			valuations_.emplace(std::move(params), b);
		}
	}
	
	bool RelationalFluent::Valuation(const std::vector<scs::Object>& objects) const {
		assert(!Is0Arity() && "Sc: Relational Fluent is inconsistent with parameters having none and some");
		if (!valuations_.contains(objects)) {
			SCS_DEBUG("The valuation of objects {} doesn't exist", ObjectVectorToString(objects));
			return false; // @Check this
		} else {
			return valuations_.at(objects);
		}
	}

	// 0-arity evaluation (no parameter fluent)
	bool RelationalFluent::Valuation() const {
		assert(Is0Arity() == true && "Sc: Relational Fluent is inconsistent with parameters having none and same");
		return valuations_.at({{"0-arity"}});
	}


	/*
	 * @brief: Performance check for any valuation of the relational fluent whether the object "o" is referenced at all inside it 
	 */
	bool RelationalFluent::ContainsObject(const Object& o) const {
		return objects_.contains(o);
	}

	bool RelationalFluent::Is0Arity() const {
		return valuations_.contains({"0-arity"});
	}

	// == Operator overloads ==

	std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent) {
		if (fluent.Is0Arity()) {
			stream << fluent.name_ << " = " << BoolToString(fluent.valuations_.at({ "0-arity" }));
			return stream;
		}

		for (auto it = fluent.valuations_.begin(); it != fluent.valuations_.end(); ++it) {
			if (it != fluent.valuations_.begin()) {
				stream << ", ";
			}
			const auto& val = *it;
			stream << fluent.name_ << "(" << ObjectVectorToString(val.first) << ")" << " = " << BoolToString(val.second);
		}

		return stream;
	}

	bool RelationalFluent::operator==(const RelationalFluent& other) const {
		return (name_ == other.name_ && valuations_ == other.valuations_);
	}

	bool RelationalFluent::operator!=(const RelationalFluent& other) const {
		return (name_ != other.name_ || valuations_ != other.valuations_);
	}

}