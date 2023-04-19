#pragma once

#include "scs/SituationCalculus/bat.h"

#include <span>

namespace scs {

	BasicActionTheory CombineBATs(const std::span<BasicActionTheory>& BATs);

}