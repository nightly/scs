#pragma once

#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/coop_matrix.h"

#include <span>

namespace scs {

	BasicActionTheory CombineBATs(const std::span<BasicActionTheory>& BATs, const CoopMatrix& mat);

}