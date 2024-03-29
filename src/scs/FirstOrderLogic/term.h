#pragma once

#include <variant>
#include <string>
#include <unordered_set>
#include <ostream>

#include "scs/FirstOrderLogic/object.h" 
#include "scs/FirstOrderLogic/variable.h"

namespace scs {

	// Term -> Variable | Constant | Function(Term, ...) [unimplemented last]

	using Term = std::variant<scs::Object, scs::Variable>;

    struct TermPrinter {
    public:
        std::ostream& os;
    public:
        TermPrinter(std::ostream& os)
            : os(os) {}

        void operator()(const scs::Object& object) {
            os << "obj(" << object << ")";
        }

        void operator()(const scs::Variable& var) {
            os << "var(" << var << ")";
        }

    };

	inline std::ostream& operator<< (std::ostream& stream, const Term& term) {
        std::visit(TermPrinter{ stream }, term);
		return stream;
	}

    inline std::string TermVectorToString(const std::vector<scs::Term>& vec) {
        std::stringstream ret;
        for (size_t i = 0; i < vec.size(); i++) {
            ret << vec[i];
            if (i != vec.size() - 1) {
                ret << ", ";
            }
        }
        return ret.str();
    }

    inline std::string TermUSetToString(const std::unordered_set<scs::Term>& terms) {
        std::stringstream ret;
        for (auto it = terms.begin(); it != terms.end(); ++it) {
            if (it != terms.begin()) {
                ret << ", ";
            }
            ret << *it;
        }
        return ret.str();
    }

}