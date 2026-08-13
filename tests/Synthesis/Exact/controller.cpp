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

TEST(ExactControllerSession, TargetControlDisambiguatesEqualRecipeRequests) {
	BasicActionTheory local;
	local.pre.emplace("ImplementLeft", Poss{true});
	local.pre.emplace("ImplementRight", Poss{true});
	local.types.emplace("ImplementLeft", ActionType::Manufacturing);
	local.types.emplace("ImplementRight", ActionType::Manufacturing);
	const Branch resource_program{
		ActionProgram{Action{"ImplementLeft"}}, ActionProgram{Action{"ImplementRight"}}};
	Resource resource{1, resource_program.clone(), std::move(local)};

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

	Controller controller;
	ArenaState environment;
	environment.owner = ArenaOwner::Environment;
	environment.recipe_control = CgState{0};
	controller.arena.initial = controller.arena.AddState(std::move(environment));
	ArenaState pending_left;
	pending_left.owner = ArenaOwner::Controller;
	pending_left.recipe_control = CgState{1};
	const ArenaStateId pending_left_id = controller.arena.AddState(std::move(pending_left));
	ArenaState pending_right;
	pending_right.owner = ArenaOwner::Controller;
	pending_right.recipe_control = CgState{2};
	const ArenaStateId pending_right_id = controller.arena.AddState(std::move(pending_right));
	ArenaState completed_left;
	completed_left.owner = ArenaOwner::Environment;
	completed_left.recipe_control = CgState{3};
	const ArenaStateId completed_left_id = controller.arena.AddState(std::move(completed_left));
	ArenaState completed_right;
	completed_right.owner = ArenaOwner::Environment;
	completed_right.recipe_control = CgState{4};
	const ArenaStateId completed_right_id = controller.arena.AddState(std::move(completed_right));
	const CompoundAction request{Action{"request"}};
	controller.arena.AddEdge({controller.arena.initial, pending_left_id, request, 0, {}});
	controller.arena.AddEdge({controller.arena.initial, pending_right_id, request, 0, {}});
	const JointAction implement_left{{ResourceStep{1, CompoundAction{Action{"ImplementLeft"}}}}};
	const JointAction implement_right{{ResourceStep{1, CompoundAction{Action{"ImplementRight"}}}}};
	const ArenaEdgeId left_response = controller.arena.AddEdge(
		{pending_left_id, completed_left_id, implement_left, 0, {}});
	const ArenaEdgeId right_response = controller.arena.AddEdge(
		{pending_right_id, completed_right_id, implement_right, 0, {}});
	controller.strategy.emplace(BudgetState{pending_left_id, 0}, left_response);
	controller.strategy.emplace(BudgetState{pending_right_id, 0}, right_response);
	controller.optimal_response_cost = 0;

	ControllerSession ambiguous{problem, controller, SequentialFreshIdentifiers()};
	EXPECT_THROW(ambiguous.Respond(request), std::invalid_argument);

	ControllerSession selected{problem, controller, SequentialFreshIdentifiers()};
	RecipeEdgeChoice choice{request};
	choice.target_control = 2;
	const ControllerResponse response = selected.Respond(choice);
	ASSERT_EQ(response.actions.size(), 1);
	EXPECT_EQ(response.actions.front().steps.front().action.Actions().front().name, "ImplementRight");
	EXPECT_EQ(selected.abstract_state(), completed_right_id);
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
