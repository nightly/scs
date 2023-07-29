#include "scs/Synthesis/global_bat.h"

#include <span>

#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/coop_matrix.h"
#include "scs/SituationCalculus/routes_matrix.h"

namespace scs {

	BasicActionTheory CombineBATs(const std::span<BasicActionTheory>& BATs, const CoopMatrix& coop, const RoutesMatrix& routes) {
		BasicActionTheory global_bat;
		Situation s_g; // The starting situation for the recipe/globally -- in essence, we need an initial description that is global across all resources
		global_bat.is_global = true;
		global_bat.SetCoopMx(coop);
		global_bat.SetRoutesMx(routes);

		for (const auto& bat_r : BATs) {
			// Add objects and relational fluents to S_r
			global_bat.objects.insert(bat_r.objects.begin(), bat_r.objects.end());
			s_g.relational_fluents_.insert(bat_r.Initial().relational_fluents_.begin(), bat_r.Initial().relational_fluents_.end());

			// Add preconditions and successors
			global_bat.pre.insert(bat_r.pre.begin(), bat_r.pre.end());
			global_bat.successors.insert(bat_r.successors.begin(), bat_r.successors.end());
			
		}

		assert(s_g.Length() == 0 && "Initial situation length must be 0 for global situation");
		global_bat.SetInitial(std::move(s_g));
		return global_bat;
	}
}