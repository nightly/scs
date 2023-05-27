#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct CgWhile : public IProgram {
	public:
		Formula condition;
		std::shared_ptr<IProgram> do_prog;
	public:
		template<typename P>
		CgWhile(const Formula& cond, const P* p)
			: condition(cond), do_prog(std::make_shared<P>(*p)) {}

		template<typename P>
		CgWhile(const Formula& cond, const P& p)
			: condition(cond), do_prog(std::make_shared<P>(p)) {}

		virtual void Decompose(Execution& exec) const override {
			// Basically, just goes into two sub executions
			// Condition, program, rest of program
			// Not condition, rest of program
		}

		bool Final(const Situation& s) const override {
			return false;
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