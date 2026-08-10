#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <utility>

#include "scs/ConGolog/Program/programs.h"
#include "scs/Synthesis/Exact/controller.h"

using namespace scs;

namespace {

	Formula Equal(Formula lhs, Formula rhs) {
		return Box<BinaryConnective>{new BinaryConnective{
			std::move(lhs), std::move(rhs), BinaryKind::Equal}};
	}

}

TEST(ExactControllerSession, HiddenRecipeBindingsAreReservedBeforeFreshAllocation) {
	BasicActionTheory local;
	local.pre.emplace("Implement", Poss{{Variable{"x"}}, true});
	local.types.emplace("Implement", ActionType::Manufacturing);
	Resource resource{1, std::make_shared<ActionProgram>(Action{"Implement", {Variable{"x"}}}),
		std::move(local)};

	FacilityComposition composition;
	composition.callbacks.possible = [](const JointAction&, const Situation&) { return true; };
	composition.callbacks.observe = [](const JointAction&) {
		return std::optional<CompoundAction>{CompoundAction{Action{"request"}}};
	};
	composition.callbacks.cost = [](const FacilityProgramStateView&, const Interpretation&,
		const JointAction&, const FacilityProgramStateView&, const Interpretation&) {
		return uint64_t{0};
	};
	SynthesisProblem problem{ComposeFacility({std::move(resource)}, std::move(composition)),
		std::make_shared<ActionProgram>(Action{"request"})};

	const Object representative_hidden = Object::Identifier("abstract-hidden");
	const Object representative_fresh = Object::Identifier("abstract-fresh");
	Controller controller;
	ArenaState environment;
	environment.owner = ArenaOwner::Environment;
	controller.arena.initial = controller.arena.AddState(environment);
	ArenaState pending;
	pending.owner = ArenaOwner::Controller;
	pending.recipe_bindings.Set(Variable{"hidden"}, representative_hidden);
	const ArenaStateId pending_id = controller.arena.AddState(std::move(pending));
	ArenaState completed;
	completed.owner = ArenaOwner::Environment;
	const ArenaStateId completed_id = controller.arena.AddState(std::move(completed));
	controller.arena.AddEdge({controller.arena.initial, pending_id,
		CompoundAction{Action{"request"}}, 0, {}});
	const JointAction representative_action{{ResourceStep{1,
		CompoundAction{Action{"Implement", {representative_fresh}}}}}};
	const ArenaEdgeId response_edge = controller.arena.AddEdge(
		{pending_id, completed_id, representative_action, 0, {}});
	controller.strategy.emplace(BudgetState{pending_id, 0}, response_edge);
	controller.optimal_response_cost = 0;

	const Object concrete_hidden = Object::Identifier("concrete-hidden");
	const Object concrete_fresh = Object::Identifier("concrete-fresh");
	bool hidden_was_reserved = false;
	ControllerSession session{problem, controller,
		[&](const ObjectSet& used) {
			hidden_was_reserved = used.contains(concrete_hidden);
			return hidden_was_reserved ? concrete_fresh : concrete_hidden;
		}};
	RecipeEdgeChoice choice{CompoundAction{Action{"request"}}};
	choice.bindings.Set(Variable{"hidden"}, concrete_hidden);
	const ControllerResponse response = session.Respond(choice);

	ASSERT_TRUE(hidden_was_reserved);
	ASSERT_EQ(response.actions.size(), 1);
	const auto& concrete_term = response.actions.front().steps.front().action.Actions().front().terms.front();
	EXPECT_EQ(std::get<Object>(concrete_term), concrete_fresh);
}

TEST(ExactModelValidation, RejectsUndeclaredRigidConstantsInSuccessorAxioms) {
	BasicActionTheory local;
	local.pre.emplace("Nop", Poss{true});
	local.types.emplace("Nop", ActionType::Nop);
	Iteration program{ActionProgram{Action{"Nop"}}};
	Resource resource{1, program.clone(), std::move(local)};

	const Variable x{"x"};
	const Object undeclared = Object::Rigid("undeclared");
	FacilityComposition composition;
	Situation initial;
	initial.AddFluent("Marked", RelationalFluent{1});
	composition.common.SetInitial(std::move(initial));
	composition.common.successors.emplace("Marked", Successor{{x}, Equal(x, undeclared)});
	composition.callbacks.observe = [](const JointAction&) {
		return std::optional<CompoundAction>{};
	};
	SynthesisProblem problem{ComposeFacility({std::move(resource)}, std::move(composition)),
		std::make_shared<ActionProgram>(Action{"request"})};
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};

	const ArenaBuildResult result = BuildArena(problem, options);
	EXPECT_EQ(result.status, ArenaBuildStatus::InvalidModel);
	EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("undeclared rigid constant") != std::string::npos;
	})) << testing::PrintToString(result.diagnostics);
}
