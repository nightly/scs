#pragma once

#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/object_universe.h"

namespace scs {

    bool ContainsTransfer(const CompoundAction& ca);

    bool ContainsRadialAndClamp(const CompoundAction& ca);

    bool PossibleTransfer(const Situation& s, const CompoundAction& ca,
        const BasicActionTheory& bat, const ObjectSet& objects);
    static bool FindOut(const CompoundAction& ca, const Action& InAct, size_t i_res,
        const BasicActionTheory& bat, const Situation& s, const ObjectSet& objects);

    bool PossibleRadial(const Situation& s, const CompoundAction& ca,
        const BasicActionTheory& bat, const ObjectSet& objects);

}
