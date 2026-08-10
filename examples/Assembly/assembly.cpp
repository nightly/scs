#include "Assembly/assembly.h"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "scs/ConGolog/Program/programs.h"
#include "scs/FirstOrderLogic/formula_utils.h"
#include "scs/FirstOrderLogic/operators.h"

namespace scs::examples {
namespace {

	const Object r1 = Object::Rigid("r1");
	const Object r2 = Object::Rigid("r2");
	const Object r3 = Object::Rigid("r3");
	const Object brass = Object::Rigid("brass");
	const Object tube = Object::Rigid("tube");
	const Object bit5 = Object::Rigid("bit5");
	const Object ok = Object::Rigid("ok");

	Formula TermFormula(const Term& term) {
		if (const auto* object = std::get_if<Object>(&term)) return *object;
		return std::get<Variable>(term);
	}

	Formula Equal(const Term& lhs, const Term& rhs, BinaryKind kind = BinaryKind::Equal) {
		return Box<BinaryConnective>{new BinaryConnective{TermFormula(lhs), TermFormula(rhs), kind}};
	}

	Formula Exists(const Variable& variable, Formula formula) {
		return Box<Quantifier>{new Quantifier{variable, std::move(formula), QuantifierKind::Existential}};
	}

	ResourceStep Step(ResourceIndex resource, Action action) {
		return {resource, CompoundAction{std::move(action)}};
	}

	std::vector<ResourceStep> Admit(const Term& x, const Term& material) {
		return {Step(1, Action{"Nop1"}), Step(2, Action{"Admit", {x, material, r2}}), Step(3, Action{"Nop3"})};
	}

	std::vector<ResourceStep> Equip() {
		return {Step(1, Action{"Nop1"}), Step(2, Action{"Nop2"}), Step(3, Action{"Equip5", {r3}})};
	}

	std::vector<ResourceStep> Drill(const Term& x) {
		return {Step(1, Action{"Clamp", {x, r1}}), Step(2, Action{"Nop2"}),
			Step(3, Action{"RadialDrill", {x, bit5, r3}})};
	}

	std::vector<ResourceStep> Release(const Term& x) {
		return {Step(1, Action{"Release", {x, r1}}), Step(2, Action{"Nop2"}), Step(3, Action{"Nop3"})};
	}

	std::vector<ResourceStep> Join(const Term& y, const Term& x) {
		return {Step(1, Action{"Nop1"}), Step(2, Action{"Nop2"}), Step(3, Action{"Join", {y, x, r3}})};
	}

	std::vector<ResourceStep> Store(const Term& x) {
		return {Step(1, Action{"Store", {x, ok, r1}}), Step(2, Action{"Nop2"}), Step(3, Action{"Nop3"})};
	}

	std::vector<ResourceStep> Transfer(const Term& x, ResourceIndex from, ResourceIndex to) {
		const std::array<Object, 4> stations{Object::Rigid("unused"), r1, r2, r3};
		std::vector<ResourceStep> result;
		for (ResourceIndex resource = 1; resource <= 3; ++resource) {
			if (resource == from) result.push_back(Step(resource, Action{"Out", {x, stations[resource]}}));
			else if (resource == to) result.push_back(Step(resource, Action{"In", {x, stations[resource]}}));
			else result.push_back(Step(resource, Action{"Nop" + std::to_string(resource)}));
		}
		return result;
	}

	Formula AdmitEvent(const Term& x, const Term& material) { return JointActionMatches(Admit(x, material)); }
	Formula EquipEvent() { return JointActionMatches(Equip()); }
	Formula DrillEvent(const Term& x) { return JointActionMatches(Drill(x)); }
	Formula ReleaseEvent(const Term& x) { return JointActionMatches(Release(x)); }
	Formula JoinEvent(const Term& y, const Term& x) { return JointActionMatches(Join(y, x)); }
	Formula StoreEvent(const Term& x) { return JointActionMatches(Store(x)); }
	Formula TransferEvent(const Term& x, ResourceIndex from, ResourceIndex to) {
		return JointActionMatches(Transfer(x, from, to));
	}

	Formula Gone(const Term& x) {
		const Variable z{"gone-z"};
		return Disjoin(StoreEvent(x), Exists(z, JoinEvent(x, z)));
	}

	Formula Incoming(const Term& x, const Term& station) {
		Formula result = false;
		for (const auto [from, to] : {std::pair{2U, 1U}, std::pair{1U, 2U},
			std::pair{2U, 3U}, std::pair{3U, 2U}}) {
			const Object target = to == 1 ? r1 : to == 2 ? r2 : r3;
			result = Disjoin(result, Conjoin(TransferEvent(x, from, to), Equal(station, target)));
		}
		return result;
	}

	Formula Outgoing(const Term& x, const Term& station) {
		Formula result = false;
		for (const auto [from, to] : {std::pair{2U, 1U}, std::pair{1U, 2U},
			std::pair{2U, 3U}, std::pair{3U, 2U}}) {
			const Object source = from == 1 ? r1 : from == 2 ? r2 : r3;
			result = Disjoin(result, Conjoin(TransferEvent(x, from, to), Equal(station, source)));
		}
		return result;
	}

	std::shared_ptr<IProgram> Act(Action action) {
		return std::make_shared<ActionProgram>(std::move(action));
	}

	std::shared_ptr<IProgram> Picked(std::vector<Variable> variables, std::shared_ptr<IProgram> body) {
		return std::make_shared<Pick>(std::move(variables), *body);
	}

	std::shared_ptr<IProgram> Choice(std::vector<std::shared_ptr<IProgram>> programs) {
		auto result = std::move(programs.front());
		for (size_t i = 1; i < programs.size(); ++i) result = std::make_shared<Branch>(*result, *programs[i]);
		return result;
	}

	std::shared_ptr<IProgram> SequenceOf(std::vector<std::shared_ptr<IProgram>> programs) {
		auto result = std::move(programs.front());
		for (size_t i = 1; i < programs.size(); ++i) result = std::make_shared<Sequence>(*result, *programs[i]);
		return result;
	}

	BasicActionTheory ResourceTheory(
		std::initializer_list<std::tuple<std::string, ActionType, size_t>> actions) {
		BasicActionTheory theory;
		for (const auto& [name, type, arity] : actions) {
			std::vector<Term> parameters;
			for (size_t i = 0; i < arity; ++i) {
				parameters.emplace_back(Variable{"arg" + std::to_string(i)});
			}
			theory.pre.emplace(name, Poss{std::move(parameters), true});
			theory.types.emplace(name, type);
		}
		return theory;
	}

	std::shared_ptr<IProgram> Resource1Program() {
		const Variable x{"x"};
		auto body = Choice({Act(Action{"Nop1"}),
			Picked({x}, Act(Action{"In", {x, r1}})),
			Picked({x}, Act(Action{"Out", {x, r1}})),
			Picked({x}, SequenceOf({Act(Action{"Clamp", {x, r1}}), Act(Action{"Release", {x, r1}})})),
			Picked({x}, Act(Action{"Store", {x, ok, r1}}))});
		return std::make_shared<Iteration>(*body);
	}

	std::shared_ptr<IProgram> Resource2Program() {
		const Variable x{"x"};
		const Variable material{"m"};
		auto body = Choice({Act(Action{"Nop2"}),
			Picked({x, material}, Act(Action{"Admit", {x, material, r2}})),
			Picked({x}, Act(Action{"In", {x, r2}})),
			Picked({x}, Act(Action{"Out", {x, r2}}))});
		return std::make_shared<Iteration>(*body);
	}

	std::shared_ptr<IProgram> Resource3Program() {
		const Variable x{"x"};
		const Variable y{"y"};
		auto body = Choice({Act(Action{"Nop3"}), Act(Action{"Equip5", {r3}}),
			Picked({x}, Act(Action{"In", {x, r3}})),
			Picked({x}, Act(Action{"Out", {x, r3}})),
			Picked({x}, Act(Action{"RadialDrill", {x, bit5, r3}})),
			Picked({x, y}, Act(Action{"Join", {y, x, r3}}))});
		return std::make_shared<Iteration>(*body);
	}

	std::shared_ptr<IProgram> RecipeProgram() {
		const Variable p{"p"};
		const Variable q{"q"};
		Formula distinct = Conjoin(Equal(p, q, BinaryKind::NotEqual),
			Conjoin(IsIdentifier(p), IsIdentifier(q)));
		auto loads = std::make_shared<Interleaved>(
			*Act(Action{"load", {p, brass}}), *Act(Action{"load", {q, tube}}));
		auto cycle = SequenceOf({std::make_shared<Check>(distinct), loads,
			Act(Action{"drill", {p, bit5}}), Act(Action{"join", {q, p}}), Act(Action{"store", {p, ok}})});
		auto selected = Picked({p, q}, cycle);
		return std::make_shared<Iteration>(*selected);
	}

	void AddEmptyFluent(Situation& initial, std::string name, size_t arity) {
		initial.AddFluent(name, RelationalFluent{arity});
	}

	FacilityComposition AssemblyComposition(AssemblyCosts costs) {
		FacilityComposition composition;
		BasicActionTheory& common = composition.common;
		for (const Object& object : {r1, r2, r3, brass, tube, bit5, ok}) {
			common.objects.emplace(object);
			common.rigid_objects.emplace(object);
		}
		for (const auto& tuple : {std::vector<Object>{r2, r1}, {r1, r2}, {r2, r3}, {r3, r2}}) {
			common.rigid.AddValuation("Route", tuple, true);
		}
		for (const auto& tuple : {std::vector<Object>{r1, r3}, {r3, r1}, {r2, r3}, {r3, r2}}) {
			common.rigid.AddValuation("Coop", tuple, true);
		}

		Situation initial;
		AddEmptyFluent(initial, "Part", 1);
		AddEmptyFluent(initial, "Material", 2);
		AddEmptyFluent(initial, "At", 2);
		AddEmptyFluent(initial, "Clamped", 1);
		AddEmptyFluent(initial, "Drilled5", 1);
		AddEmptyFluent(initial, "Assembled", 1);
		AddEmptyFluent(initial, "Equipped5", 1);
		common.SetInitial(std::move(initial));

		const Variable x{"x"};
		const Variable y{"y"};
		const Variable material{"m"};
		const Variable station{"r"};
		const Variable admitted_material{"admit-m"};
		common.successors.emplace("Part", Successor{{x},
			Disjoin(Exists(admitted_material, AdmitEvent(x, admitted_material)),
				Conjoin(cv(), Negate(Gone(x))))});
		common.successors.emplace("Material", Successor{{x, material},
			Disjoin(AdmitEvent(x, material), Conjoin(cv(), Negate(Gone(x))))});
		common.successors.emplace("At", Successor{{x, station},
			Disjoin(Conjoin(Exists(admitted_material, AdmitEvent(x, admitted_material)), Equal(station, r2)),
				Disjoin(Incoming(x, station), Conjoin(cv(),
					Conjoin(Negate(Outgoing(x, station)), Negate(Gone(x))))))});
		common.successors.emplace("Clamped", Successor{{x},
			Disjoin(DrillEvent(x), Conjoin(cv(),
				Conjoin(Negate(ReleaseEvent(x)), Negate(Gone(x))))) });
		common.successors.emplace("Drilled5", Successor{{x},
			Disjoin(DrillEvent(x), Conjoin(cv(), Negate(Gone(x))))});
		common.successors.emplace("Assembled", Successor{{x},
			Disjoin(Exists(y, JoinEvent(y, x)), Conjoin(cv(), Negate(Gone(x))))});
		common.successors.emplace("Equipped5", Successor{{station},
			Disjoin(Conjoin(EquipEvent(), Equal(station, r3)), cv())});

		composition.callbacks.may_be_possible = [](const JointAction& joint) {
			if (joint.steps.size() != 3) return false;
			std::array<std::string_view, 4> names;
			for (const auto& step : joint.steps) {
				if (step.resource < 1 || step.resource > 3 || step.action.Actions().size() != 1) return false;
				names[step.resource] = step.action.Actions().front().name;
			}
			const auto nop = [&](ResourceIndex resource) {
				return names[resource] == "Nop" + std::to_string(resource);
			};
			if (nop(1) && nop(2) && nop(3)) return true;
			if (nop(1) && names[2] == "Admit" && nop(3)) return true;
			if (nop(1) && nop(2) && names[3] == "Equip5") return true;
			if (names[1] == "Clamp" && nop(2) && names[3] == "RadialDrill") return true;
			if (names[1] == "Release" && nop(2) && nop(3)) return true;
			if (nop(1) && nop(2) && names[3] == "Join") return true;
			if (names[1] == "Store" && nop(2) && nop(3)) return true;
			ResourceIndex from = 0;
			ResourceIndex to = 0;
			for (ResourceIndex resource = 1; resource <= 3; ++resource) {
				if (names[resource] == "Out") from = resource;
				else if (names[resource] == "In") to = resource;
				else if (!nop(resource)) return false;
			}
			return (from == 2 && (to == 1 || to == 3))
				|| (to == 2 && (from == 1 || from == 3));
		};

		composition.callbacks.possible = [](const JointAction& joint, const Situation& state) {
			const auto event = [&]() -> std::optional<std::tuple<std::string, std::vector<Object>>> {
				if (joint.steps.size() != 3) return std::nullopt;
				std::array<const Action*, 4> actions{};
				for (const auto& step : joint.steps) {
					if (step.resource < 1 || step.resource > 3 || step.action.Actions().size() != 1) return std::nullopt;
					actions[step.resource] = &step.action.Actions().front();
				}
				if (!actions[1] || !actions[2] || !actions[3]) return std::nullopt;
				auto objects = [](const Action& action) -> std::optional<std::vector<Object>> {
					std::vector<Object> result;
					for (const Term& term : action.terms) {
						const auto* object = std::get_if<Object>(&term);
						if (!object) return std::nullopt;
						result.push_back(*object);
					}
					return result;
				};
				auto args1 = objects(*actions[1]);
				auto args2 = objects(*actions[2]);
				auto args3 = objects(*actions[3]);
				if (!args1 || !args2 || !args3) return std::nullopt;
				const auto nop = [](const Action& action, ResourceIndex i) {
					return action.name == "Nop" + std::to_string(i) && action.terms.empty();
				};
				if (nop(*actions[1], 1) && nop(*actions[2], 2) && nop(*actions[3], 3)) return {{"nop", {}}};
				if (nop(*actions[1], 1) && actions[2]->name == "Admit" && args2->size() == 3
					&& (*args2)[2] == r2 && nop(*actions[3], 3)) return {{"admit", *args2}};
				if (nop(*actions[1], 1) && nop(*actions[2], 2) && actions[3]->name == "Equip5"
					&& *args3 == std::vector<Object>{r3}) return {{"equip", *args3}};
				if (actions[1]->name == "Clamp" && args1->size() == 2 && (*args1)[1] == r1
					&& nop(*actions[2], 2) && actions[3]->name == "RadialDrill" && args3->size() == 3
					&& (*args3)[0] == (*args1)[0] && (*args3)[1] == bit5 && (*args3)[2] == r3) {
					return {{"drill", {*args1->begin()}}};
				}
				if (actions[1]->name == "Release" && args1->size() == 2 && (*args1)[1] == r1
					&& nop(*actions[2], 2) && nop(*actions[3], 3)) return {{"release", {(*args1)[0]}}};
				if (nop(*actions[1], 1) && nop(*actions[2], 2) && actions[3]->name == "Join"
					&& args3->size() == 3 && (*args3)[2] == r3) return {{"join", *args3}};
				if (actions[1]->name == "Store" && args1->size() == 3 && (*args1)[1] == ok && (*args1)[2] == r1
					&& nop(*actions[2], 2) && nop(*actions[3], 3)) return {{"store", *args1}};

				const std::array<Object, 4> station{Object::Rigid("unused"), r1, r2, r3};
				ResourceIndex from = 0;
				ResourceIndex to = 0;
				Object part;
				for (ResourceIndex i = 1; i <= 3; ++i) {
					if (actions[i]->name == "Out" && (i != 1 || actions[i]->name != "Nop1")) {
						if (from != 0 || (i != 1 && i != 2 && i != 3)) return std::nullopt;
						auto args = objects(*actions[i]);
						if (!args || args->size() != 2 || (*args)[1] != station[i]) return std::nullopt;
						from = i;
						if (to != 0 && part != (*args)[0]) return std::nullopt;
						part = (*args)[0];
					} else if (actions[i]->name == "In") {
						if (to != 0) return std::nullopt;
						auto args = objects(*actions[i]);
						if (!args || args->size() != 2 || (*args)[1] != station[i]) return std::nullopt;
						to = i;
						if (from != 0 && part != (*args)[0]) return std::nullopt;
						part = (*args)[0];
					} else if (!nop(*actions[i], i)) {
						return std::nullopt;
					}
				}
				if (from != 0 && to != 0 && from != to) return {{"transfer", {part, station[from], station[to]}}};
				return std::nullopt;
			}();
			if (!event) return false;
			const auto& [kind, args] = *event;
			const auto holds = [&](std::string_view fluent, const std::vector<Object>& tuple) {
				const auto found = state.Fluents().find(std::string(fluent));
				return found != state.Fluents().end() && found->second.Valuation(tuple);
			};
			const auto station_empty = [&](const Object& station) {
				const auto& at = state.Fluents().at("At");
				return std::ranges::none_of(at.TrueTuples(), [&](const auto& tuple) {
					return tuple.size() == 2 && tuple[1] == station;
				});
			};
			const auto reach = [&](const Object& part, const Object& station) {
				if (holds("At", {part, station})) return true;
				for (const Object& other : {r1, r2, r3}) {
					if (holds("At", {part, other}) && ((station == r1 && other == r3)
						|| (station == r3 && (other == r1 || other == r2))
						|| (station == r2 && other == r3))) return true;
				}
				return false;
			};
			if (kind == "nop") return true;
			if (kind == "admit") {
				return args[0].IsIdentifier() && (args[1] == brass || args[1] == tube) && !holds("Part", {args[0]})
					&& state.Fluents().at("Part").TrueTuples().size() < 2 && station_empty(r2);
			}
			if (kind == "equip") return !holds("Equipped5", {r3});
			if (kind == "drill") return holds("Part", {args[0]}) && holds("At", {args[0], r1})
				&& holds("Equipped5", {r3}) && !holds("Clamped", {args[0]});
			if (kind == "release") return holds("Part", {args[0]}) && holds("At", {args[0], r1})
				&& holds("Clamped", {args[0]});
			if (kind == "join") return args[0] != args[1] && holds("Part", {args[0]})
				&& holds("Part", {args[1]}) && holds("Drilled5", {args[1]})
				&& reach(args[0], r3) && reach(args[1], r3);
			if (kind == "store") return holds("Part", {args[0]}) && holds("At", {args[0], r1})
				&& holds("Assembled", {args[0]}) && !holds("Clamped", {args[0]});
			if (kind == "transfer") {
				const bool route = (args[1] == r2 && (args[2] == r1 || args[2] == r3))
					|| (args[2] == r2 && (args[1] == r1 || args[1] == r3));
				return route && holds("Part", {args[0]}) && holds("At", {args[0], args[1]})
					&& !holds("Clamped", {args[0]}) && station_empty(args[2]);
			}
			return false;
		};

		composition.callbacks.observe = [](const JointAction& joint) -> std::optional<CompoundAction> {
			if (joint.steps.size() != 3) return std::nullopt;
			const auto find = [&](ResourceIndex resource) -> const Action* {
				const auto step = std::ranges::find(joint.steps, resource, &ResourceStep::resource);
				return step != joint.steps.end() && step->action.Actions().size() == 1
					? &step->action.Actions().front() : nullptr;
			};
			const Action* a1 = find(1);
			const Action* a2 = find(2);
			const Action* a3 = find(3);
			if (!a1 || !a2 || !a3) return std::nullopt;
			if (a2->name == "Admit" && a2->terms.size() == 3) {
				return CompoundAction{Action{"load", {a2->terms[0], a2->terms[1]}}};
			}
			if (a1->name == "Clamp" && a3->name == "RadialDrill" && a3->terms.size() == 3) {
				return CompoundAction{Action{"drill", {a3->terms[0], a3->terms[1]}}};
			}
			if (a3->name == "Join" && a3->terms.size() == 3) {
				return CompoundAction{Action{"join", {a3->terms[0], a3->terms[1]}}};
			}
			if (a1->name == "Store" && a1->terms.size() == 3) {
				return CompoundAction{Action{"store", {a1->terms[0], a1->terms[1]}}};
			}
			return std::nullopt;
		};

		composition.callbacks.cost = [costs](const FacilityProgramStateView&, const Situation&,
			const JointAction& joint, const FacilityProgramStateView&, const Situation&) {
			uint64_t cost = costs.base;
			for (const auto& step : joint.steps) {
				for (const Action& action : step.action.Actions()) {
					if (action.name.starts_with("Nop")) cost += costs.nop;
					else if (action.name == "In" || action.name == "Out") cost += costs.transfer;
					else if (action.name == "Equip5") cost += costs.preparatory;
					else cost += costs.manufacturing;
				}
			}
			return cost;
		};
		return composition;
	}

}

	SynthesisProblem MakeAssemblyProblem(AssemblyCosts costs) {
		Resource resource1{1, Resource1Program(), ResourceTheory({
			{"Nop1", ActionType::Nop, 0}, {"In", ActionType::Transfer, 2},
			{"Out", ActionType::Transfer, 2}, {"Clamp", ActionType::Manufacturing, 2},
			{"Release", ActionType::Manufacturing, 2}, {"Store", ActionType::Manufacturing, 3}})};
		Resource resource2{2, Resource2Program(), ResourceTheory({
			{"Nop2", ActionType::Nop, 0}, {"Admit", ActionType::Manufacturing, 3},
			{"In", ActionType::Transfer, 2}, {"Out", ActionType::Transfer, 2}})};
		Resource resource3{3, Resource3Program(), ResourceTheory({
			{"Nop3", ActionType::Nop, 0}, {"Equip5", ActionType::Preparatory, 1},
			{"In", ActionType::Transfer, 2}, {"Out", ActionType::Transfer, 2},
			{"RadialDrill", ActionType::Manufacturing, 3}, {"Join", ActionType::Manufacturing, 3}})};
		return {ComposeFacility({std::move(resource1), std::move(resource2), std::move(resource3)},
			AssemblyComposition(costs)), RecipeProgram()};
	}

	SynthesisOptions AssemblySynthesisOptions(WorklistOrder worklist) {
		SynthesisOptions options;
		options.backend = FaithfulAbstractionBackend{2, worklist};
		return options;
	}

}
