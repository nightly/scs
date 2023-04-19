#pragma once

#include <string>
#include <unordered_set>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/term.h"

namespace scs {

	struct Action {
		std::string name;
		std::vector<scs::Term> parameters; 
		
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
		// We assume the UNA
		size_t operator() (const scs::Action& act) const {
			return hash<std::string>()(act.name);
		}
	};

}