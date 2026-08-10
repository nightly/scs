#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <boost/container_hash/hash.hpp>

#include "scs/FirstOrderLogic/object.h"

namespace scs {

	class RigidRelation {
	public:
		using Tuple = std::vector<Object>;
		using Valuations = ankerl::unordered_dense::map<Tuple, bool, boost::hash<Tuple>>;

		RigidRelation() = default;
		explicit RigidRelation(size_t arity);

		size_t Arity() const;
		const Valuations& ExplicitValuations() const;
		std::optional<bool> ExplicitValuation(const Tuple& tuple) const;
		bool Valuation(const Tuple& tuple) const;
		bool Valuation() const;
		void AddValuation(const Tuple& tuple, bool value);
		void AddValuation(Tuple&& tuple, bool value);
		void AddValuation(bool value);

	private:
		size_t arity_ = 0;
		Valuations valuations_;
	};

	class RigidDatabase {
	public:
		using Relations = ankerl::unordered_dense::map<std::string, RigidRelation>;

		RigidRelation& Declare(const std::string& name, size_t arity);
		void AddValuation(const std::string& name, const RigidRelation::Tuple& tuple, bool value);
		void AddValuation(const std::string& name, RigidRelation::Tuple&& tuple, bool value);
		void Merge(const RigidDatabase& other);

		bool contains(const std::string& name) const;
		const RigidRelation& at(const std::string& name) const;
		RigidRelation& at(const std::string& name);
		auto begin() const { return relations_.begin(); }
		auto end() const { return relations_.end(); }
		auto begin() { return relations_.begin(); }
		auto end() { return relations_.end(); }

	private:
		Relations relations_;
	};

}
