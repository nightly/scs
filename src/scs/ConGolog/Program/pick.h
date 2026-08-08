#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	struct Pick : public IProgram {
		std::vector<Variable> args;
		std::shared_ptr<IProgram> p;

		Pick(const std::vector<Variable>& args, const IProgram* p)
			: args(args), p(p->clone()) {}

		Pick(const std::vector<Variable>& args, const IProgram& p)
			: args(args), p(p.clone()) {}

		Pick(std::vector<Variable>&& args, const IProgram& p)
			: args(std::move(args)), p(p.clone()) {}

		Pick(std::initializer_list<Variable> args, const IProgram& p)
			: args(args), p(p.clone()) {}

		Pick(const std::unordered_set<Variable>& args, const IProgram& p)
			: args(args.begin(), args.end()), p(p.clone()) {
			std::ranges::sort(this->args, {}, &Variable::name);
		}

		Pick(const std::unordered_set<Variable>& args, const IProgram* p)
			: args(args.begin(), args.end()), p(p->clone()) {
			std::ranges::sort(this->args, {}, &Variable::name);
		}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Pick>(*this);
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Pick>";
			os << "	<From> ";
			for (auto it = args.begin(); it != args.end(); ++it) {
				if (it != args.begin()) {
					os << ",";
				}
				os << *it;
			}
			os << "\n";
			os << " <To> " << *p << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Pick& prog) {
		prog.Print(os);
		return os;
	}


}
