#include "utils.h"

#include <ankerl/unordered_dense.h>

namespace scs {

	std::vector<CgTransition> TransitionsFromPermutations(const std::vector<std::vector<scs::Object>>& perm,
	const CgTransition& original) {
		std::vector<CgTransition> ret;

		for (const auto& p : perm) {
			CgTransition trans = original;
			assert(trans.HasVariables() && "called TransitionsFromPermutations despite having no variables in the original transition");
				
			// Create variable mapping from this permutation order
			ankerl::unordered_dense::map<Variable, Object> mapping;
			for (size_t i = 0; i < trans.vars.size(); ++i) {
				mapping[trans.vars[i]] = p[i];
			}

			// Modify condition with substituion @Incomplete/not needed
			// ...
			
			// Replace action with substitution
			for (auto& act : trans.act.Actions()) {
				for (auto& term : act.terms) {
					if (auto p = std::get_if<Variable>(&term)) {
						if (mapping.contains(*p)) {
							term = mapping.at(*p);
						}
					}
				}
			}

			ret.emplace_back(trans);
		}

		return ret;
	}

}