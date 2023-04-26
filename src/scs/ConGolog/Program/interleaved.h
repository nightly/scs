#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Interleaved concurrency (p || q)
	struct Interleaved : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		template<typename P, typename Q>
		Interleaved(const P* p, const Q* q)
			: p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		Interleaved(const P& p, const Q& q)
			: p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}

		virtual std::vector<ActionState> Decompose(const Situation& s) const override {
			std::vector<ActionState> ret;

			return ret;
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Interleaved>" << p << " || " << q;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Interleaved& prog) {
		prog.Print(os);
		return os;
	}

}