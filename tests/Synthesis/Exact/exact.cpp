#include <gtest/gtest.h>

#include <limits>

#include "scs/ConGolog/Program/programs.h"
#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/FirstOrderLogic/formula_utils.h"
#include "scs/FirstOrderLogic/operators.h"
#include "scs/Synthesis/Exact/controller.h"
#include "scs/Synthesis/Exact/export.h"
#include "Assembly/assembly.h"

using namespace scs;

namespace {

	Formula Equal(Formula lhs, Formula rhs, BinaryKind kind = BinaryKind::Equal) {
		return Box<BinaryConnective>{new BinaryConnective{std::move(lhs), std::move(rhs), kind}};
	}

	Formula Exists(Variable variable, Formula child) {
		return Box<Quantifier>{new Quantifier{variable, child, QuantifierKind::Existential}};
	}

	SynthesisProblem SimpleExactProblem() {
		BasicActionTheory local;
		local.pre.emplace("Nop1", Poss{true});
		local.pre.emplace("Work", Poss{{Variable{"x"}}, true});
		local.types.emplace("Nop1", ActionType::Nop);
		local.types.emplace("Work", ActionType::Manufacturing);

		ActionProgram nop{Action{"Nop1"}};
		ActionProgram work{Action{"Work", {Variable{"x"}}}};
		Pick choose_work{{Variable{"x"}}, work};
		Branch choice{nop, choose_work};
		Iteration resource_program{choice};
		Resource resource{1, resource_program.clone(), std::move(local)};

		FacilityComposition composition;
		composition.callbacks.possible = [](const JointAction&, const Situation&) { return true; };
		composition.callbacks.observe = [](const JointAction& action) -> std::optional<CompoundAction> {
			for (const auto& component : action.Flatten().Actions()) {
				if (component.name == "Work") return CompoundAction{component};
			}
			return std::nullopt;
		};

		ActionProgram requested{Action{"Work", {Variable{"part"}}}};
		Pick recipe_pick{{Variable{"part"}}, requested};
		return {ComposeFacility({std::move(resource)}, std::move(composition)), recipe_pick.clone()};
	}

	SynthesisProblem FreshInternalProblem() {
		BasicActionTheory local;
		local.pre.emplace("Internal", Poss{{Variable{"z"}}, true});
		local.pre.emplace("Work", Poss{{Variable{"x"}}, true});
		local.types.emplace("Internal", ActionType::Manufacturing);
		local.types.emplace("Work", ActionType::Manufacturing);
		const Variable x{"x"};
		const Variable z{"z"};
		Check distinct{Equal(x, z, BinaryKind::NotEqual)};
		Sequence actions{ActionProgram{Action{"Internal", {z}}}, ActionProgram{Action{"Work", {x}}}};
		Sequence guarded{distinct, actions};
		Pick choose{{x, z}, guarded};
		Iteration resource_program{choose};
		Resource resource{1, resource_program.clone(), std::move(local)};

		FacilityComposition composition;
		composition.callbacks.possible = [](const JointAction&, const Interpretation&) { return true; };
		composition.callbacks.observe = [](const JointAction& action) -> std::optional<CompoundAction> {
			const Action& local_action = action.steps.front().action.Actions().front();
			return local_action.name == "Work" ? std::optional<CompoundAction>{CompoundAction{local_action}}
				: std::nullopt;
		};
		Pick request{{Variable{"part"}}, ActionProgram{Action{"Work", {Variable{"part"}}}}};
		Iteration recipe{request};
		return {ComposeFacility({std::move(resource)}, std::move(composition)), recipe.clone()};
	}

	SynthesisProblem ZeroCostInternalCycleProblem() {
		BasicActionTheory local;
		local.pre.emplace("InternalA", Poss{true});
		local.pre.emplace("InternalB", Poss{true});
		local.pre.emplace("Work", Poss{{Variable{"x"}}, true});
		local.types.emplace("InternalA", ActionType::Manufacturing);
		local.types.emplace("InternalB", ActionType::Manufacturing);
		local.types.emplace("Work", ActionType::Manufacturing);
		Sequence internal{ActionProgram{Action{"InternalA"}}, ActionProgram{Action{"InternalB"}}};
		Pick work{{Variable{"x"}}, ActionProgram{Action{"Work", {Variable{"x"}}}}};
		Iteration resource_program{Branch{internal, work}};
		Resource resource{1, resource_program.clone(), std::move(local)};

		FacilityComposition composition;
		composition.callbacks.possible = [](const JointAction&, const Interpretation&) { return true; };
		composition.callbacks.observe = [](const JointAction& action) -> std::optional<CompoundAction> {
			const Action& local_action = action.steps.front().action.Actions().front();
			return local_action.name == "Work"
				? std::optional<CompoundAction>{CompoundAction{local_action}} : std::nullopt;
		};
		composition.callbacks.cost = [](const FacilityProgramStateView&, const Interpretation&,
			const JointAction&, const FacilityProgramStateView&, const Interpretation&) { return uint64_t{0}; };
		Pick request{{Variable{"part"}}, ActionProgram{Action{"Work", {Variable{"part"}}}}};
		return {ComposeFacility({std::move(resource)}, std::move(composition)), request.clone()};
	}

	SynthesisProblem EqualPrefixRecipeProblem() {
		BasicActionTheory local;
		for (const std::string name : {"a", "b", "c"}) {
			local.pre.emplace(name, Poss{true});
			local.types.emplace(name, ActionType::Manufacturing);
		}
		const Branch alternatives{ActionProgram{Action{"a"}},
			Branch{ActionProgram{Action{"b"}}, ActionProgram{Action{"c"}}}};
		const Iteration resource_program{alternatives};
		Resource resource{1, resource_program.clone(), std::move(local)};

		FacilityComposition composition;
		composition.callbacks.possible = [](const JointAction&, const Interpretation&) { return true; };
		composition.callbacks.observe = [](const JointAction& action) {
			return std::optional<CompoundAction>{action.steps.front().action};
		};
		const Sequence left{ActionProgram{Action{"a"}}, ActionProgram{Action{"b"}}};
		const Sequence right{ActionProgram{Action{"a"}}, ActionProgram{Action{"c"}}};
		const Branch recipe{left, right};
		return {ComposeFacility({std::move(resource)}, std::move(composition)), recipe.clone()};
	}

	Arena ResponseTradeoffArena() {
		Arena arena;
		ArenaState environment;
		environment.owner = ArenaOwner::Environment;
		arena.initial = arena.AddState(environment);
		ArenaState goal;
		goal.owner = ArenaOwner::Goal;
		arena.goal = arena.AddState(goal);
		ArenaState lose;
		lose.owner = ArenaOwner::Lose;
		arena.lose = arena.AddState(lose);
		ArenaState first;
		first.owner = ArenaOwner::Controller;
		const auto c0 = arena.AddState(first);
		ArenaState second;
		second.owner = ArenaOwner::Controller;
		const auto c1 = arena.AddState(second);
		arena.AddEdge({arena.goal, arena.goal, AuxiliaryLabel::Sink, 0, {}});
		arena.AddEdge({arena.lose, arena.lose, AuxiliaryLabel::Sink, 0, {}});
		arena.AddEdge({arena.initial, arena.goal, AuxiliaryLabel::Stop, 0, {}});
		arena.AddEdge({arena.initial, c0, CompoundAction{Action{"request"}}, 0, {}});
		arena.AddEdge({c0, c1, JointAction{}, 1, {}});
		arena.AddEdge({c1, arena.initial, JointAction{}, 10, {}});
		arena.AddEdge({c0, arena.initial, JointAction{}, 5, {}});
		return arena;
	}

}

TEST(InfiniteDomainEvaluation, AddsAnonymousEqualityTypesWithoutClosingTheDomain) {
	BasicActionTheory bat;
	const Object rigid = Object::Rigid("rigid");
	bat.objects.emplace(rigid);
	bat.rigid_objects.emplace(rigid);
	Situation state;
	bat.SetInitial(state);
	ObjectSet finite{rigid};

	Formula one_fresh = Exists(Variable{"x"},
		Equal(Variable{"x"}, rigid, BinaryKind::NotEqual));
	Domain finite_domain{state, bat, finite};
	finite_domain.semantics = DomainSemantics::Finite;
	EXPECT_FALSE(EvaluateFormula(one_fresh, finite_domain));

	Domain infinite_domain{state, bat, finite};
	infinite_domain.semantics = DomainSemantics::InfiniteGeneric;
	EXPECT_TRUE(EvaluateFormula(one_fresh, infinite_domain));

	Formula two_distinct = Exists(Variable{"x"}, Exists(Variable{"y"},
		Equal(Variable{"x"}, Variable{"y"}, BinaryKind::NotEqual)));
	EXPECT_TRUE(EvaluateFormula(two_distinct, infinite_domain));

	const Object reserved_collision = Object::Identifier("@scs-anonymous-0");
	ObjectSet colliding_carrier{rigid, reserved_collision};
	Domain colliding_domain{state, bat, colliding_carrier};
	colliding_domain.semantics = DomainSemantics::InfiniteGeneric;
	Formula two_more_identifiers = Exists(Variable{"x"}, Exists(Variable{"y"},
		Conjoin(IsIdentifier(Variable{"x"}), Conjoin(IsIdentifier(Variable{"y"}),
			Conjoin(Equal(Variable{"x"}, reserved_collision, BinaryKind::NotEqual),
				Conjoin(Equal(Variable{"y"}, reserved_collision, BinaryKind::NotEqual),
					Equal(Variable{"x"}, Variable{"y"}, BinaryKind::NotEqual)))))));
	EXPECT_TRUE(EvaluateFormula(two_more_identifiers, colliding_domain));

	FirstOrderAssignment outer;
	outer.Set(Variable{"x"}, rigid);
	Formula scoped = Conjoin(one_fresh, Equal(Variable{"x"}, rigid));
	EXPECT_TRUE(EvaluateFormula(scoped, infinite_domain, outer));
}

TEST(ExactSynthesis, FaithfulArenaSolvesValidatesAndLiftsFreshRequests) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_EQ(result.optimal_response_cost, 2);
	ASSERT_TRUE(result.controller);
	EXPECT_TRUE(result.validation.valid);
	EXPECT_EQ(result.controller->arena.bounds.support, 2);

	ControllerSession session{problem, *result.controller,
		SequentialFreshIdentifiers("controller-")};
	const Object customer_part = Object::Identifier("customer-93842");
	const auto response = session.Respond(CompoundAction{Action{"Work", {customer_part}}});
	ASSERT_EQ(response.actions.size(), 1);
	EXPECT_EQ(response.cost, 2);
	ASSERT_EQ(response.actions.front().steps.size(), 1);
	EXPECT_EQ(std::get<Object>(response.actions.front().steps.front().action.Actions().front().terms.front()),
		customer_part);
	session.Stop();
	EXPECT_EQ(session.abstract_state(), result.controller->arena.goal);
}

TEST(ExactSynthesis, LiftedResponseMustPreserveWhetherTheActionIsVisible) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_TRUE(result.controller);

	problem.facility.callbacks.observe = [](const JointAction&) {
		return std::optional<CompoundAction>{};
	};
	ControllerSession session{problem, *result.controller,
		SequentialFreshIdentifiers("lifted-")};
	EXPECT_THROW(session.Respond(CompoundAction{
		Action{"Work", {Object::Identifier("customer-part")}}}), std::runtime_error);
}

TEST(ExactSynthesis, EqualRequestPrefixesRetainEnvironmentEdgeChoice) {
	auto problem = EqualPrefixRecipeProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{0, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_TRUE(result.controller);
	ASSERT_TRUE(result.validation.valid);

	const CompoundAction first_request{Action{"a"}};
	std::vector<size_t> target_controls;
	for (const ArenaEdgeId edge_id : result.controller->arena.outgoing.at(result.controller->arena.initial)) {
		const ArenaEdge& edge = result.controller->arena.edges.at(edge_id);
		if (const auto* request = std::get_if<CompoundAction>(&edge.label);
			request != nullptr && *request == first_request) {
			target_controls.push_back(result.controller->arena.states.at(edge.target).recipe_control.n);
		}
	}
	ASSERT_EQ(target_controls.size(), 2);
	EXPECT_NE(target_controls[0], target_controls[1]);

	ControllerSession ambiguous{problem, *result.controller, SequentialFreshIdentifiers()};
	EXPECT_THROW(ambiguous.Respond(first_request), std::invalid_argument);

	ControllerSession selected{problem, *result.controller, SequentialFreshIdentifiers()};
	RecipeEdgeChoice choice{first_request};
	choice.target_control = target_controls.front();
	const ControllerResponse first_response = selected.Respond(choice);
	ASSERT_EQ(first_response.actions.size(), 1);
	EXPECT_EQ(first_response.actions.front().steps.front().action, first_request);

	const ArenaState& selected_state = result.controller->arena.states.at(selected.abstract_state());
	ASSERT_EQ(selected_state.owner, ArenaOwner::Environment);
	std::vector<CompoundAction> continuations;
	for (const ArenaEdgeId edge_id : result.controller->arena.outgoing.at(selected.abstract_state())) {
		const ArenaEdge& edge = result.controller->arena.edges.at(edge_id);
		if (const auto* request = std::get_if<CompoundAction>(&edge.label)) continuations.push_back(*request);
	}
	ASSERT_EQ(continuations.size(), 1);
	EXPECT_TRUE(continuations.front() == CompoundAction{Action{"b"}}
		|| continuations.front() == CompoundAction{Action{"c"}});
}

TEST(ExactSynthesis, FiniteAndFaithfulBackendsAgree) {
	auto problem = SimpleExactProblem();
	SynthesisOptions finite;
	finite.backend = FiniteDomainBackend{ObjectSet{Object::Identifier("a"), Object::Identifier("b")}};
	const auto grounded = Synthesise(problem, finite);
	ASSERT_EQ(grounded.status, SynthesisStatus::Winning);

	for (const WorklistOrder order : {WorklistOrder::BreadthFirst,
		WorklistOrder::LowerCostFirst, WorklistOrder::Greedy}) {
		SynthesisOptions abstract;
		abstract.backend = FaithfulAbstractionBackend{1, order};
		const auto symbolic = Synthesise(problem, abstract);
		ASSERT_EQ(symbolic.status, SynthesisStatus::Winning);
		EXPECT_EQ(symbolic.optimal_response_cost, grounded.optimal_response_cost);
		EXPECT_TRUE(symbolic.validation.valid);
	}
}

TEST(ExactSynthesis, FiniteArenaWitnessesRemainValidAcrossStateGrowth) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FiniteDomainBackend{ObjectSet{
		Object::Identifier("a"), Object::Identifier("b"), Object::Identifier("c"),
		Object::Identifier("d")}};
	options.validate_controller = false;
	const auto built = BuildArena(problem, options);
	ASSERT_EQ(built.status, ArenaBuildStatus::Complete);
	ASSERT_GT(built.arena.states.size(), 4);
	for (const auto& edge : built.arena.edges) {
		EXPECT_TRUE(IsBijectionWitness(edge.witness));
		if (edge.source == built.arena.goal || edge.source == built.arena.lose) continue;
		const ObjectRenaming expected = [&] {
			ObjectRenaming identity;
			for (const Object& object : EdgeSupport(built.arena.states[edge.source], edge.label,
				built.arena.states[edge.target])) {
				identity.emplace_back(object, object);
			}
			return identity;
		}();
		EXPECT_EQ(edge.witness, expected);
	}
}

TEST(ExactSynthesis, UniversalRecipeBranchingMakesAnUnserviceableRequestLosing) {
	auto problem = SimpleExactProblem();
	ActionProgram missing{Action{"Missing"}};
	Branch adversarial{*problem.recipe, missing};
	problem.recipe = adversarial.clone();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	EXPECT_EQ(Synthesise(problem, options).status, SynthesisStatus::Losing);
}

TEST(ExactSynthesis, FinalRecipeStatesOfferStopAndContinuingRequests) {
	auto problem = SimpleExactProblem();
	Iteration repeated{*problem.recipe};
	problem.recipe = repeated.clone();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.validate_controller = false;
	const auto built = BuildArena(problem, options);
	ASSERT_EQ(built.status, ArenaBuildStatus::Complete);
	bool stop = false;
	bool request = false;
	for (const ArenaEdgeId edge_id : built.arena.outgoing[built.arena.initial]) {
		const ArenaLabel& label = built.arena.edges[edge_id].label;
		stop |= std::get_if<AuxiliaryLabel>(&label)
			&& std::get<AuxiliaryLabel>(label) == AuxiliaryLabel::Stop;
		request |= std::holds_alternative<CompoundAction>(label);
	}
	EXPECT_TRUE(stop);
	EXPECT_TRUE(request);
}

TEST(ExactSynthesis, DiagnosesSampledCallbackEquivarianceViolations) {
	auto problem = SimpleExactProblem();
	problem.facility.callbacks.possible = [](const JointAction& action, const Interpretation&) {
		for (const Action& component : action.Flatten().Actions()) {
			if (component.name == "Work") {
				const auto& object = std::get<Object>(component.terms.front());
				return object.name().starts_with("@scs-id-");
			}
		}
		return true;
	};
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	EXPECT_EQ(result.status, SynthesisStatus::InvalidModel);
	EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("equivariance") != std::string::npos;
	}));
}

TEST(ExactSynthesis, RejectsStaticRenameableIdentifierConstantsInFaithfulModels) {
	auto problem = SimpleExactProblem();
	problem.recipe = std::make_shared<ActionProgram>(
		Action{"Work", {Object::Identifier("distinguished")}});
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	EXPECT_EQ(result.status, SynthesisStatus::InvalidModel);
	EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("identifier constants") != std::string::npos;
	}));

	auto rigid_problem = SimpleExactProblem();
	rigid_problem.facility.bat.rigid.AddValuation(
		"Distinguished", {Object::Identifier("distinguished")}, true);
	const auto rigid_result = Synthesise(rigid_problem, options);
	EXPECT_EQ(rigid_result.status, SynthesisStatus::InvalidModel);
	EXPECT_TRUE(std::ranges::any_of(rigid_result.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("renameable identifier") != std::string::npos;
	})) << testing::PrintToString(rigid_result.diagnostics);
}

TEST(ExactSynthesis, HonoursCancellationDeadlines) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.deadline = std::chrono::steady_clock::now();
	EXPECT_EQ(Synthesise(problem, options).status, SynthesisStatus::Cancelled);
}

TEST(ExactSynthesis, SupportsZeroCostControllerTransitions) {
	auto problem = SimpleExactProblem();
	problem.facility.callbacks.cost = [](const FacilityProgramStateView&, const Interpretation&,
		const JointAction&, const FacilityProgramStateView&, const Interpretation&) { return uint64_t{0}; };
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	EXPECT_EQ(result.status, SynthesisStatus::Winning);
	EXPECT_EQ(result.optimal_response_cost, 0);
	EXPECT_TRUE(result.validation.valid);
	EXPECT_GT(result.statistics.arena.removed_self_loops, 0);
}

TEST(ExactSynthesis, SupportsAZeroBoundWhenNoIdentifierPersistsInDynamicState) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{0, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_TRUE(result.controller);
	EXPECT_EQ(result.controller->arena.bounds.active_domain, 0);
	EXPECT_EQ(result.optimal_response_cost, 2);
	EXPECT_TRUE(result.validation.valid);
}

TEST(ExactSynthesis, FiniteBackendDoesNotRequireIdentifierEquivariance) {
	auto problem = SimpleExactProblem();
	problem.facility.callbacks.cost = [](const FacilityProgramStateView&, const Interpretation&,
		const JointAction& joint, const FacilityProgramStateView&, const Interpretation&) {
		for (const Action& action : joint.Flatten().Actions()) {
			if (action.name == "Work") {
				return std::get<Object>(action.terms.front()).name() == "a" ? uint64_t{2} : uint64_t{3};
			}
		}
		return uint64_t{0};
	};
	SynthesisOptions options;
	options.backend = FiniteDomainBackend{
		ObjectSet{Object::Identifier("a"), Object::Identifier("b")}};
	const auto result = Synthesise(problem, options);
	EXPECT_EQ(result.status, SynthesisStatus::Winning);
	EXPECT_EQ(result.optimal_response_cost, 3);
}

TEST(ExactSynthesis, ControllerExportsAreDeterministic) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_TRUE(result.controller);
	const std::string dot = ControllerToGraphViz(*result.controller);
	const std::string tikz = ControllerToTikz(*result.controller);
	EXPECT_EQ(dot, ControllerToGraphViz(*result.controller));
	EXPECT_EQ(tikz, ControllerToTikz(*result.controller));
	EXPECT_NE(dot.find("digraph Controller"), std::string::npos);
	EXPECT_NE(tikz.find("\\begin{tikzpicture}"), std::string::npos);
}

TEST(ExactSynthesis, ControllerSessionAllocatesLifetimeFreshInternalIdentifiers) {
	auto problem = FreshInternalProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	const auto result = Synthesise(problem, options);
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_TRUE(result.controller);
	EXPECT_EQ(result.optimal_response_cost, 4);
	ControllerSession session{problem, *result.controller, SequentialFreshIdentifiers("created-")};
	ObjectSet created;
	for (size_t cycle = 0; cycle < 3; ++cycle) {
		const Object request = Object::Identifier("request-" + std::to_string(cycle));
		const auto response = session.Respond(CompoundAction{Action{"Work", {request}}});
		ASSERT_EQ(response.actions.size(), 2);
		const Object internal = std::get<Object>(
			response.actions.front().steps.front().action.Actions().front().terms.front());
		EXPECT_NE(internal, request);
		EXPECT_TRUE(created.emplace(internal).second);
	}
	session.Stop();
}

TEST(ExactFacility, ResourceIndicesSurviveFlatteningAndJointMatching) {
	const JointAction action{{ResourceStep{1, CompoundAction{Action{"Nop"}}},
		ResourceStep{2, CompoundAction{Action{"Nop"}}}}};
	CompoundAction indexed = action.IndexedFlatten();
	ASSERT_EQ(indexed.Actions().size(), 2);
	EXPECT_EQ(indexed.Actions()[0].name, IndexedActionName(1, "Nop"));
	EXPECT_EQ(indexed.Actions()[1].name, IndexedActionName(2, "Nop"));
	FirstOrderAssignment assignment;
	assignment.Set(Variable{"a"}, indexed);
	EXPECT_TRUE(EvaluateFormula(JointActionMatches(action.steps), Domain{}, assignment));
	indexed.AppendAction(Action{"extra"});
	assignment.Set(Variable{"a"}, indexed);
	EXPECT_FALSE(EvaluateFormula(JointActionMatches(action.steps), Domain{}, assignment));
}

TEST(ExactFacility, JointCallbackCanEnableAnOperationWhoseLocalPartsAreImpossible) {
	BasicActionTheory first;
	first.pre.emplace("Together", Poss{false});
	first.types.emplace("Together", ActionType::Manufacturing);
	BasicActionTheory second = first;
	Resource r1{1, std::make_shared<ActionProgram>(Action{"Together"}), std::move(first)};
	Resource r2{2, std::make_shared<ActionProgram>(Action{"Together"}), std::move(second)};
	FacilityComposition composition;
	composition.callbacks.observe = [](const JointAction&) { return std::optional<CompoundAction>{}; };
	composition.callbacks.possible = [](const JointAction& action, const Interpretation&) {
		return action.steps.size() == 2;
	};
	const Facility facility = ComposeFacility({std::move(r1), std::move(r2)}, std::move(composition));
	const JointAction joint{{ResourceStep{1, CompoundAction{Action{"Together"}}},
		ResourceStep{2, CompoundAction{Action{"Together"}}}}};
	EXPECT_TRUE(facility.Possible(joint, facility.bat.Initial(), {}, DomainSemantics::Finite));
	const JointAction incomplete{{ResourceStep{1, CompoundAction{Action{"Together"}}}}};
	EXPECT_FALSE(facility.Possible(incomplete, facility.bat.Initial(), {}, DomainSemantics::Finite));
	const JointAction unknown_resource{{ResourceStep{1, CompoundAction{Action{"Together"}}},
		ResourceStep{3, CompoundAction{Action{"Together"}}}}};
	EXPECT_FALSE(facility.Possible(
		unknown_resource, facility.bat.Initial(), {}, DomainSemantics::Finite));
	const JointAction nonground{{ResourceStep{1, CompoundAction{Action{"Together", {Variable{"x"}}}}},
		ResourceStep{2, CompoundAction{Action{"Together"}}}}};
	EXPECT_FALSE(facility.Possible(nonground, facility.bat.Initial(), {}, DomainSemantics::Finite));
	const JointAction wrong_arity{{ResourceStep{1, CompoundAction{Action{"Together", {Object::Rigid("x")}}}},
		ResourceStep{2, CompoundAction{Action{"Together"}}}}};
	EXPECT_FALSE(facility.Possible(wrong_arity, facility.bat.Initial(), {}, DomainSemantics::Finite));
}

TEST(InfiniteDomainEvaluation, CompoundPreconditionsRetainInfiniteDomainSemantics) {
	BasicActionTheory bat;
	const Object rigid = Object::Rigid("rigid");
	bat.objects.emplace(rigid);
	bat.rigid_objects.emplace(rigid);
	bat.pre.emplace("A", Poss{Exists(Variable{"x"},
		Equal(Variable{"x"}, rigid, BinaryKind::NotEqual))});
	Situation state;
	EXPECT_TRUE(state.Possible(CompoundAction{Action{"A"}}, bat));
}

TEST(ExactFacility, CompositionRejectsSharedSsaAndExplicitDatabaseConflicts) {
	const auto resource = [](ResourceIndex index, bool rigid_value) {
		BasicActionTheory bat;
		bat.pre.emplace("Nop", Poss{true});
		bat.types.emplace("Nop", ActionType::Nop);
		Situation initial;
		initial.AddFluent("Shared", RelationalFluent{0});
		bat.SetInitial(initial);
		bat.successors.emplace("Shared", Successor{{}, true});
		bat.rigid.AddValuation("R", {Object::Rigid("c")}, rigid_value);
		return Resource{index, std::make_shared<ActionProgram>(Action{"Nop"}), std::move(bat)};
	};
	FacilityComposition composition;
	composition.callbacks.observe = [](const JointAction&) { return std::optional<CompoundAction>{}; };
	EXPECT_THROW(ComposeFacility({resource(1, true), resource(2, true)}, composition), std::invalid_argument);

	FacilityComposition with_override = composition;
	with_override.combined_successors.emplace("Shared", Successor{{}, true});
	EXPECT_THROW(ComposeFacility({resource(1, true), resource(2, false)}, std::move(with_override)),
		std::invalid_argument);
}

TEST(ExactAbstraction, EqualityTypesIncludeRepeatedAndDistinctFreshBindings) {
	BasicActionTheory local;
	local.pre.emplace("Nop", Poss{true});
	local.types.emplace("Nop", ActionType::Nop);
	Iteration resource_program{ActionProgram{Action{"Nop"}}};
	FacilityComposition composition;
	composition.callbacks.observe = [](const JointAction&) { return std::optional<CompoundAction>{}; };
	const Variable x{"x"};
	const Variable y{"y"};
	Pick request{{x, y}, ActionProgram{Action{"Request", {x, y}}}};
	SynthesisProblem problem{ComposeFacility({Resource{1, resource_program.clone(), std::move(local)}},
		std::move(composition)), request.clone()};
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.validate_controller = false;
	const auto built = BuildArena(problem, options);
	ASSERT_EQ(built.status, ArenaBuildStatus::Complete);
	bool repeated = false;
	bool distinct = false;
	for (const ArenaEdgeId edge_id : built.arena.outgoing[built.arena.initial]) {
		const auto* label = std::get_if<CompoundAction>(&built.arena.edges[edge_id].label);
		if (!label) continue;
		const auto& terms = label->Actions().front().terms;
		const Object& lhs = std::get<Object>(terms[0]);
		const Object& rhs = std::get<Object>(terms[1]);
		repeated |= lhs == rhs;
		distinct |= lhs != rhs;
	}
	EXPECT_TRUE(repeated);
	EXPECT_TRUE(distinct);
}

TEST(ExactSolver, OptimizesARecurrentLifetimeTradeoffAndResetsAtEnvironment) {
	const auto result = SolveArena(ResponseTradeoffArena());
	ASSERT_EQ(result.status, SynthesisStatus::Winning);
	ASSERT_EQ(result.optimal_response_cost, 5);
	ASSERT_TRUE(result.controller);
	EXPECT_EQ(result.statistics.greedy_upper_bound, 10);
	EXPECT_GE(result.statistics.theoretical_upper_bound, 5);
}

TEST(ExactSolver, RejectsControllerOnlyResponseCycles) {
	Arena arena = ResponseTradeoffArena();
	const ArenaStateId c0 = 3;
	arena.outgoing[c0].clear();
	arena.AddEdge({c0, c0, JointAction{}, 1, {}});
	const auto result = SolveArena(std::move(arena));
	EXPECT_EQ(result.status, SynthesisStatus::Losing);
}

TEST(ExactSolver, SaturatesAnOverflowingTheoreticalBoundWhenAFeasibleBoundFits) {
	Arena arena = ResponseTradeoffArena();
	for (ArenaEdge& edge : arena.edges) {
		if (edge.cost > 0) edge.cost = std::numeric_limits<uint64_t>::max();
	}
	const auto result = SolveArena(std::move(arena));
	EXPECT_EQ(result.status, SynthesisStatus::Winning);
	EXPECT_EQ(result.optimal_response_cost, std::numeric_limits<uint64_t>::max());
	EXPECT_EQ(result.statistics.theoretical_upper_bound, std::numeric_limits<uint64_t>::max());
}

TEST(ExactValidator, DetectsSemanticAndWitnessMutations) {
	auto problem = SimpleExactProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.validate_controller = false;
	const auto solved = Synthesise(problem, options);
	ASSERT_TRUE(solved.controller);

	Controller wrong_cost = *solved.controller;
	const auto selected = wrong_cost.strategy.begin()->second;
	++wrong_cost.arena.edges[selected].cost;
	EXPECT_FALSE(ValidateController(problem, wrong_cost).valid);

	Controller bad_witness = *solved.controller;
	bad_witness.arena.edges[selected].witness = {
		{Object::Identifier("x"), Object::Identifier("z")},
		{Object::Identifier("y"), Object::Identifier("z")}};
	EXPECT_FALSE(ValidateController(problem, bad_witness).valid);

	Controller omitted = *solved.controller;
	omitted.arena.outgoing[omitted.arena.initial].clear();
	EXPECT_FALSE(ValidateController(problem, omitted).valid);
}

TEST(ExactValidator, RejectsZeroCostControllerOnlyResponseCycles) {
	auto problem = ZeroCostInternalCycleProblem();
	SynthesisOptions options;
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.validate_controller = false;
	const auto solved = Synthesise(problem, options);
	ASSERT_EQ(solved.status, SynthesisStatus::Winning);
	ASSERT_TRUE(solved.controller);
	Controller cyclic = *solved.controller;

	ArenaStateId pending = cyclic.arena.lose;
	for (const ArenaEdgeId edge_id : cyclic.arena.outgoing[cyclic.arena.initial]) {
		if (std::holds_alternative<CompoundAction>(cyclic.arena.edges[edge_id].label)) {
			pending = cyclic.arena.edges[edge_id].target;
			break;
		}
	}
	ASSERT_NE(pending, cyclic.arena.lose);
	std::optional<ArenaEdgeId> enter_cycle;
	for (ArenaEdgeId edge_id = 0; edge_id < cyclic.arena.edges.size(); ++edge_id) {
		const ArenaEdge& edge = cyclic.arena.edges[edge_id];
		const auto* joint = std::get_if<JointAction>(&edge.label);
		if (edge.source == pending && joint != nullptr
			&& joint->Flatten().Actions().front().name == "InternalA") {
			enter_cycle = edge_id;
			break;
		}
	}
	ASSERT_TRUE(enter_cycle);
	cyclic.arena.outgoing[pending].push_back(*enter_cycle);
	cyclic.strategy[{pending, 0}] = *enter_cycle;
	const ValidationReport report = ValidateController(problem, cyclic);
	EXPECT_FALSE(report.valid);
	EXPECT_TRUE(std::ranges::any_of(report.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("response cycle") != std::string::npos;
	}));
}

TEST(AssemblyExact, HasPaperBoundsAndOptimalRecurrentResponse) {
	auto problem = examples::MakeAssemblyProblem();
	const auto result = Synthesise(problem, examples::AssemblySynthesisOptions());
	ASSERT_EQ(result.status, SynthesisStatus::Winning) << testing::PrintToString(result.diagnostics);
	EXPECT_EQ(result.statistics.bounds.active_domain, 2);
	EXPECT_EQ(result.statistics.bounds.recipe_live, 2);
	EXPECT_EQ(result.statistics.bounds.facility_live, 1);
	EXPECT_EQ(result.statistics.bounds.recipe_action_arity, 2);
	EXPECT_EQ(result.statistics.bounds.support, 7);
	EXPECT_EQ(result.statistics.bounds.fresh_edge, 5);
	EXPECT_EQ(result.statistics.bounds.pool, 19);
	EXPECT_EQ(result.optimal_response_cost, 10);
	EXPECT_TRUE(result.validation.valid);

	ControllerSession session{problem, *result.controller, SequentialFreshIdentifiers("assembly-controller-")};
	for (size_t cycle = 0; cycle < 5; ++cycle) {
		const Object p = Object::Identifier("concrete-p-" + std::to_string(cycle));
		const Object q = Object::Identifier("concrete-q-" + std::to_string(cycle));
		RecipeEdgeChoice first{CompoundAction{Action{"load", {p, Object::Rigid("brass")}}}};
		first.bindings.Set(Variable{"p"}, p);
		first.bindings.Set(Variable{"q"}, q);
		const auto first_response = session.Respond(first);
		EXPECT_LE(first_response.cost, 10);
		EXPECT_LE(RenameableActiveDomainSize(session.concrete_state()), 2);
		for (const CompoundAction& request : {
			CompoundAction{Action{"load", {q, Object::Rigid("tube")}}},
			CompoundAction{Action{"drill", {p, Object::Rigid("bit5")}}},
			CompoundAction{Action{"join", {q, p}}},
			CompoundAction{Action{"store", {p, Object::Rigid("ok")}}}}) {
			const auto response = session.Respond(request);
			EXPECT_LE(response.cost, 10);
			EXPECT_LE(RenameableActiveDomainSize(session.concrete_state()), 2);
		}
		EXPECT_EQ(RenameableActiveDomainSize(session.concrete_state()), 0);
	}
	session.Stop();
	EXPECT_EQ(session.abstract_state(), result.controller->arena.goal);
}

TEST(AssemblyExact, EveryWorklistClosesToTheSameOptimum) {
	for (const WorklistOrder order : {WorklistOrder::LowerCostFirst, WorklistOrder::Greedy}) {
		auto problem = examples::MakeAssemblyProblem();
		auto options = examples::AssemblySynthesisOptions(order);
		options.validate_controller = false;
		const auto result = Synthesise(problem, options);
		ASSERT_EQ(result.status, SynthesisStatus::Winning);
		EXPECT_EQ(result.optimal_response_cost, 10);
		EXPECT_EQ(result.statistics.bounds.pool, 19);
	}
}

TEST(AssemblyExact, RejectsAnActiveDomainBoundBelowTheReachableSupport) {
	auto problem = examples::MakeAssemblyProblem();
	auto options = examples::AssemblySynthesisOptions();
	options.backend = FaithfulAbstractionBackend{1, WorklistOrder::BreadthFirst};
	options.validate_controller = false;
	const auto result = Synthesise(problem, options);
	EXPECT_EQ(result.status, SynthesisStatus::InvalidModel);
	EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const std::string& diagnostic) {
		return diagnostic.find("active-domain bound") != std::string::npos;
	}));
}
