#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

#include "scs/Memory/box.h"

namespace scs {

	// Non-deterministic branching (p | q)
	struct Branch : public IProgram {
		const IProgram* p;
		const IProgram* q;


		Branch(const IProgram* p, const IProgram* q) 
			: p(p), q(q) {}


		bool Final(const Situation& s) const override {
			return p->Final(s) || q->Final(s);
		}

		bool Trans(const Situation& s) const override {
			return p->Trans(s) || q->Final(s);
		}

		virtual std::vector<Configuration> Transmute(const Situation& s) const override {
			std::vector<Configuration> ret;
			// @Correctness
			if (p->Trans(s)) {
				ret.emplace_back(p->Transmute(s)[0]);
			}
			if (q->Trans(s)) {
				ret.emplace_back(q->Transmute(s)[0]);
			}
			return ret;
		}

	};

}