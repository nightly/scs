#pragma once

#include <string>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/term.h"

namespace scs {

	struct Action {
		std::string name;
		std::vector<scs::Term> terms; 

		Action() = default;
		Action(const std::string& name);
		Action(std::string&& name);
		Action(const std::string& name, const std::vector<Term>& parameters);
		Action(const std::string& name, std::vector<Term>&& parameters);
		Action(std::string&& name, std::vector<Term>&& parameters);

		Action(const Action& other);
		Action& operator=(const Action& other);
		Action(Action&& other) noexcept;
		Action& operator=(Action&& other) noexcept;

		bool HasParams() const;

		bool operator==(const Action& other) const;
		bool operator!=(const Action& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Action& action);

	};

}

namespace std {

	template <>
	struct std::hash<scs::Action> {
		size_t operator() (const scs::Action& act) const {
			size_t seed = 0;
			boost::hash_combine(seed, std::hash<std::string>()(act.name));
			for (const auto& t : act.terms) {
				boost::hash_combine(seed, std::hash<scs::Term>()(t));
			}
			return seed;
		}
	};

}