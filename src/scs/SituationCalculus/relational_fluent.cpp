#include "scs/SituationCalculus/relational_fluent.h"

#include <string>
#include <unordered_set>
#include <ostream>
#include <stdexcept>

#include "scs/Common/strings.h"
#include "scs/Common/log.h"

namespace scs {

	// == Ctors, dtors ==
	RelationalFluent::RelationalFluent(size_t arity) : arity_(arity) {}

	// == Getters == 

	size_t RelationalFluent::Arity() const {
		return arity_;
	}

	const RelationalFluent::TupleSet& RelationalFluent::TrueTuples() const {
		return true_tuples_;
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
		if (arity_ != 0 && arity_ != 8080) {
			throw std::invalid_argument("Adding valuation to fluent that has previously set different arity");
		}
		arity_ = 0;
		if (b) {
			true_tuples_.emplace();
		} else {
			true_tuples_.erase(Tuple{});
		}
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(const std::vector<Object>& params, bool b) {
		if (params.size() != arity_ && arity_ != 8080) {
			throw std::invalid_argument("Adding valuation to fluent that has previously set different arity");
		}
		arity_ = params.size();
		if (b) {
			true_tuples_.emplace(params);
		} else {
			true_tuples_.erase(params);
		}
	}

	/*
	 * @brief: Will add or update valuation
	 */
	void RelationalFluent::AddValuation(std::vector<Object>&& params, bool b) {
		if (params.size() != arity_ && arity_ != 8080) {
			throw std::invalid_argument("Adding valuation to fluent that has previously set different arity");
		}
		arity_ = params.size();
		if (b) {
			true_tuples_.emplace(std::move(params));
		} else {
			true_tuples_.erase(params);
		}
	}

	void RelationalFluent::Clear() {
		true_tuples_.clear();
	}
	
	bool RelationalFluent::Valuation(const std::vector<scs::Object>& objects) const {
		if (objects.size() != arity_ && arity_ != 8080) {
			throw std::invalid_argument("Searching valuation in Relational Fluent that has different arity than stored");
		}

		if (!true_tuples_.contains(objects)) {
			SCS_DEBUG("The valuation of objects {} doesn't exist", ObjectVectorToString(objects));
			return false; // @Assumption: closed world assumption
		}
		return true;
	}

	// 0-arity evaluation (no parameter fluent)
	bool RelationalFluent::Valuation() const {
		if (arity_ != 0) {
			throw std::invalid_argument("Looking for 0-arity valuation in Relational Fluent that has non 0-arity stores");
		}
		return true_tuples_.contains(Tuple{});
	}


	// == Operator overloads ==

	std::ostream& operator<< (std::ostream& stream, const RelationalFluent& fluent) {
		if (fluent.Arity() == 0) {
			stream << "Fluent" << " = " << BoolToString(fluent.Valuation());
			return stream;
		}

		for (auto it = fluent.true_tuples_.begin(); it != fluent.true_tuples_.end(); ++it) {
			if (it != fluent.true_tuples_.begin()) {
				stream << ", ";
			}
			stream << "(" << ObjectVectorToString(*it) << ") = true";
		}

		return stream;
	}

	bool RelationalFluent::operator==(const RelationalFluent& other) const {
		return arity_ == other.arity_ && true_tuples_ == other.true_tuples_;
	}

	bool RelationalFluent::operator!=(const RelationalFluent& other) const {
		return !(*this == other);
	}

	size_t RelationalFluentHash::operator()(const RelationalFluent& fluent) const {
		size_t valuations_hash = 0;
		for (const auto& parameters : fluent.TrueTuples()) {
			size_t valuation_hash = 0;
			boost::hash_range(valuation_hash, parameters.begin(), parameters.end());
			valuations_hash += valuation_hash;
		}

		size_t seed = 0;
		boost::hash_combine(seed, fluent.Arity());
		boost::hash_combine(seed, fluent.TrueTuples().size());
		boost::hash_combine(seed, valuations_hash);
		return seed;
	}

}
