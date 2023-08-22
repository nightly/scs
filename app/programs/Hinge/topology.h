#include "Hinge/hinge.h"
#include "Hinge/Full/recipe.h"

using namespace scs;
using namespace scs::examples;

void CliHingeTopology() {
	std::vector<CharacteristicGraph> graphs;
	scs::BasicActionTheory global;
	CharacteristicGraph graph_recipe;

	auto resource1 = HingeResource1();
	graphs.emplace_back(resource1.program, ProgramType::Resource);

	auto resource2 = HingeResource2();
	graphs.emplace_back(resource2.program, ProgramType::Resource);

	auto resource3 = HingeResource3();
	graphs.emplace_back(resource3.program, ProgramType::Resource);

	auto common = HingeCommon();
	auto common_bat = HingeCommonBAT();

	size_t num = 1000;
	for (size_t i = 0; i < num; ++i) {
		SCS_INFO("Iteration");
		CompleteTopology ct(&graphs, true);
	}
}
