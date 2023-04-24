#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/ConGolog/Program/check.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct CgWhile : public IProgram {
	public:
		Check condition;
		std::shared_ptr<IProgram> do_prog;
	public:
		template<typename P>
		CgWhile(const Formula& cond, const P* p)
			: condition(cond), do_prog(std::make_shared<P>(*p)) {}

		template<typename P>
		CgWhile(const Formula& cond, const P& p)
			: condition(cond), do_prog(std::make_shared<P>(p)) {}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			return ret;
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