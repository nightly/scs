#include "scs/SituationCalculus/poss_mappings.h"

#include "scs/Common/log.h"
#include "scs/FirstOrderLogic/evaluator.h"

namespace scs {

	bool PossibleTransfer(const Situation& s, const CompoundAction& ca, const BasicActionTheory& bat) {
		if (bat.RoutesMx().IsEmpty()) {
			SCS_CRITICAL("In and Out actions found in resources but Routes Matrix is empty.");
			return false;
		}
		size_t num_in(0), num_out(0);
		for (const auto& a : ca.Actions()) {
			if (a.name == "In") {
				num_in++;
			} else if (a.name == "Out") {
				num_out++;
			}
		}
		if (num_in != num_out) {
			return false;
		}

		ankerl::unordered_dense::set<Object> parts;
		for (size_t i = 0; i < ca.Actions().size(); ++i) {
			const auto& act = ca.Actions().at(i);
			if (act.name == "In") {
				if (parts.contains(std::get<Object>(act.terms[0]))) {
					return false;
				}
				parts.emplace(std::get<Object>(act.terms[0]));
				if (!s.Possible(act, bat)) {
					return false;
				}
				bool possible_out = FindOut(ca, act, i, bat, s);
				if (!possible_out) {
					return false;
				}
			} else if (act.name == "Out") {
				continue; // already handled by In
			} else {
				bool local = s.Possible(act, bat);
				if (!local) {
					return false;
				}
			}
		}
		return true;
	}

	static bool FindOut(const CompoundAction& ca, const Action& InAct, size_t i, const BasicActionTheory& bat, const Situation& s) {
		for (size_t j = 0; j < ca.Actions().size(); ++j) {
			const auto& act = ca.Actions().at(j);
			if (act.name == "Out") {
				const auto& part = std::get<Object>(act.terms[0]);
				if (part.name() != std::get<Object>(InAct.terms[0]).name()) {
					return false;
				}
				if (bat.RoutesMx().Lookup((i + 1), (j + 1)) && s.Possible(act, bat)) {
					return true;
				}
			}
		}
		return false;
	}

	// @CLeanup =  better way to do mappings...
	bool PossibleRadial(const Situation& s, const CompoundAction& ca, const BasicActionTheory& bat) {
		Object o;
		size_t drill_n;
		for (size_t i = 0; i < ca.Actions().size(); ++i) {
			const auto& act = ca.Actions()[i];
			if ((act.name == "In") || (act.name == "Out")) {
				return false;
			} else if (act.name == "Clamp") {
				bool local = s.Possible(act, bat);
				if (!local) {
					return false;
				}
				o = std::get<Object>(act.terms.at(0));
			} else if (act.name == "RadialDrill") {
				drill_n = i;
			} else {
				bool local = s.Possible(act, bat);
				
				if (!local) {
					return false;
				}
			}
		}
		const Action& a = ca.Actions().at(drill_n);
		if (o != std::get<Object>(a.terms[0])) {
			return false;
		}

		FirstOrderAssignment assignment;
		Formula pre_drill = Predicate("material", { Variable{"part"} }) && 
			Predicate("equipped_bit", { Variable{"bit"}, Variable{"i"}}) &&
			Predicate("suitable", { Variable{"bit"}, Variable{"diameter"} });
		std::vector<Term> terms = {Variable{"part"}, Variable{"bit"}, Variable{"diameter"}, Variable{"i"}};
		for (size_t i = 0; i < a.terms.size(); ++i) {
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&terms.at(i))) {
				const auto& obj = std::get<Object>(a.terms[i]);
				assignment.Set(*var_ptr, obj);
			}
		}

		scs::Evaluator eval{ {s, bat, bat.CoopMx(), bat.RoutesMx()}, assignment };
		bool drill = std::visit(eval, pre_drill);
		return drill;
	}

}