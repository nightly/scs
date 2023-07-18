#pragma once

#include <string>
#include <vector>
#include <set>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	struct Pick : public IProgram {
		std::unordered_set<Variable> args;
		std::shared_ptr<IProgram> p;

		Pick(std::unordered_set<Variable>& args, const IProgram* p)
			: args(args), p(p->clone()) {}

		Pick(std::unordered_set<Variable>& args, const IProgram& p)
			: args(args), p(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Pick>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {

		}

		virtual std::shared_ptr<IProgram> Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return std::make_shared<Nil>();
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