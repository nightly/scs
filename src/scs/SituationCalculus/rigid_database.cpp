#include "scs/SituationCalculus/rigid_database.h"

#include <stdexcept>

namespace scs {
namespace {

	void CheckArity(size_t expected, size_t actual) {
		if (expected != actual) {
			throw std::invalid_argument("Rigid-relation tuple has inconsistent arity");
		}
	}

}

	RigidRelation::RigidRelation(size_t arity) : arity_(arity) {}

	size_t RigidRelation::Arity() const {
		return arity_;
	}

	const RigidRelation::Valuations& RigidRelation::ExplicitValuations() const {
		return valuations_;
	}

	std::optional<bool> RigidRelation::ExplicitValuation(const Tuple& tuple) const {
		CheckArity(arity_, tuple.size());
		const auto found = valuations_.find(tuple);
		return found == valuations_.end() ? std::nullopt : std::optional<bool>{found->second};
	}

	bool RigidRelation::Valuation(const Tuple& tuple) const {
		return ExplicitValuation(tuple).value_or(false);
	}

	bool RigidRelation::Valuation() const {
		return Valuation({});
	}

	void RigidRelation::AddValuation(const Tuple& tuple, bool value) {
		CheckArity(arity_, tuple.size());
		const auto [found, inserted] = valuations_.try_emplace(tuple, value);
		if (!inserted && found->second != value) {
			throw std::invalid_argument("Rigid relation has inconsistent explicit truth values");
		}
	}

	void RigidRelation::AddValuation(Tuple&& tuple, bool value) {
		CheckArity(arity_, tuple.size());
		const auto found = valuations_.find(tuple);
		if (found != valuations_.end()) {
			if (found->second != value) {
				throw std::invalid_argument("Rigid relation has inconsistent explicit truth values");
			}
			return;
		}
		valuations_.emplace(std::move(tuple), value);
	}

	void RigidRelation::AddValuation(bool value) {
		AddValuation({}, value);
	}

	RigidRelation& RigidDatabase::Declare(const std::string& name, size_t arity) {
		const auto [found, inserted] = relations_.try_emplace(name, arity);
		if (!inserted && found->second.Arity() != arity) {
			throw std::invalid_argument("Rigid relation '" + name + "' has inconsistent arity");
		}
		return found->second;
	}

	void RigidDatabase::AddValuation(const std::string& name,
		const RigidRelation::Tuple& tuple, bool value) {
		Declare(name, tuple.size()).AddValuation(tuple, value);
	}

	void RigidDatabase::AddValuation(const std::string& name,
		RigidRelation::Tuple&& tuple, bool value) {
		const size_t arity = tuple.size();
		Declare(name, arity).AddValuation(std::move(tuple), value);
	}

	void RigidDatabase::Merge(const RigidDatabase& other) {
		for (const auto& [name, relation] : other) {
			auto& target = Declare(name, relation.Arity());
			for (const auto& [tuple, value] : relation.ExplicitValuations()) {
				target.AddValuation(tuple, value);
			}
		}
	}

	bool RigidDatabase::contains(const std::string& name) const {
		return relations_.contains(name);
	}

	const RigidRelation& RigidDatabase::at(const std::string& name) const {
		return relations_.at(name);
	}

	RigidRelation& RigidDatabase::at(const std::string& name) {
		return relations_.at(name);
	}

}
