#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"
#include "scs/ConGolog/Program/nil.h"

namespace scs {

	struct CgWhile : public IProgram {
	public:
		Formula condition;
		std::shared_ptr<IProgram> do_prog;
	public:
		CgWhile(const Formula& cond, const IProgram* p)
			: condition(cond), do_prog(p->clone()) {}

		CgWhile(const Formula& cond, const IProgram& p)
			: condition(cond), do_prog(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<CgWhile>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {

		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return {};
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<While>" << condition;
			os << " <Do>" << do_prog;
			os << "\n";
			return os;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const CgWhile& prog) {
		prog.Print(os);
		return os;
	}

}