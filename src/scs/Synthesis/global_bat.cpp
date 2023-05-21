#include "scs/Synthesis/global_bat.h"

#include <span>

#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/coop_matrix.h"

namespace scs {

	BasicActionTheory CombineBATs(const std::span<BasicActionTheory>& BATs, const CoopMatrix& mat) {
		BasicActionTheory global_bat;
		Situation s_g; // The starting situation for the recipe/globally -- in essence, we need an initial description that is global across all resources
		global_bat.mat = mat;

		for (const auto& bat_r : BATs) {
			// Add objects and relational fluents to S_r
			for (const auto& obj : bat_r.Initial().objects) {
				s_g.objects.emplace(obj);
			}
			for (const auto& rf : bat_r.Initial().relational_fluents_) {
				s_g.relational_fluents_.emplace(rf);
			}

			// Add preconditions and successors
			for (const auto& pre : bat_r.pre) {
				global_bat.pre.emplace(pre.first, pre.second);
			}
			for (const auto& ssa : bat_r.successors) {
				global_bat.successors.emplace(ssa.first, ssa.second);
			}
		}

		assert(s_g.Length() == 0 && "Initial situation length must be 0 for global situation");
		global_bat.SetInitial(std::move(s_g));
		return global_bat;
	}
}