#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

#include "scs/Memory/box.h"

namespace scs {

	// Non-deterministic branching (p | q)
	struct Branch : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		template<typename P, typename Q>
		Branch(const P* p, const Q* q)
			: p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		Branch(const P& p, const Q& q)
			: p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}


		bool Final(const Situation& s) const override {
			return p->Final(s) || q->Final(s);
		}

		bool Trans(const Situation& s) const override {
			return p->Trans(s) || q->Final(s);
		}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;
			if (p->Trans(s)) {
				auto decomposition = p->Decompose(s);
				ret.insert(ret.end(), decomposition.begin(), decomposition.end());
			}
			if (q->Trans(s)) {
				auto decomposition = q->Decompose(s);
				ret.insert(ret.end(), decomposition.begin(), decomposition.end());
			}
			return ret;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<NonDet>" << *p << " | " << *q;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Branch& br) {
		br.Print(os);
		return os;
	}

}