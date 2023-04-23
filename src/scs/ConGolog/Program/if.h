#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	struct CgIf : public IProgram {
	public:
		Check check;
		std::shared_ptr<IProgram> p; // true block
		std::shared_ptr<IProgram> q; // else block

	public:
		template<typename P, typename Q>
		CgIf(const Formula& check, const P* p, const Q* q)
			: check(check), p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		CgIf(const Formula& check, const P& p, const Q& q)
			: check(check), p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}


		bool Final(const Situation& s) const override {
			return false;
		}

		bool Trans(const Situation& s) const override {
			return false;
		}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			return ret;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<If>" << " " << check;
			os << "	<Then>";
			os << p;
			os << "	<Else>";
			os << q;
			os << "\n";
			return os;
		}


	};

	inline std::ostream& operator<< (std::ostream& os, const CgIf& prog) {
		prog.Print(os);
		return os;
	}


}