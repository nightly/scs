#include <gtest/gtest.h>

#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"
#include "Hinge/common.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

#include "scs/Synthesis/Topology/Complete/utils.h"
#include "scs/Synthesis/Topology/export.h"

using namespace scs::examples;
using namespace scs;

static std::string export_dir = "../../tests/Synthesis/Topology/Exports/Hinge/";

TEST(TopologyShuffle, Transitions) {
	scs::Resource resource1, resource2, resource4;
	std::vector<CharacteristicGraph> graphs;

	resource1 = HingeResource1();
	graphs.emplace_back(resource1.program, ProgramType::Resource);
	resource2 = HingeResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);
	resource4 = HingeResource4();
	graphs.emplace_back(resource4.program, ProgramType::Resource);

	CompleteTopology ct(&graphs);
	// 	std::cout << ct.lts();
	// ------------------------------------------------------------ //
	ExportTopology(ct, "TopologyOriginal", export_dir);
	ShuffleComplete(ct);
	ExportTopology(ct, "TopologyShuffled", export_dir);
}