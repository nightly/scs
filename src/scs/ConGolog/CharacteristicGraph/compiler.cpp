#include "scs/ConGolog/CharacteristicGraph/compiler.h"

#include <algorithm>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/FirstOrderLogic/formula_utils.h"

namespace scs {

	namespace {

		enum class ProgramKind {
			Nil,
			Action,
			Check,
			Sequence,
			Branch,
			Pick,
			Iteration,
			Interleaved,
			Simultaneous,
		};

		struct Program;
		using ProgramPtr = std::shared_ptr<const Program>;

		struct Program {
			ProgramKind kind = ProgramKind::Nil;
			CompoundAction action;
			Formula formula = false;
			std::vector<Variable> variables;
			ProgramPtr left;
			ProgramPtr right;
		};

		ProgramPtr MakeNil() {
			static const auto nil = std::make_shared<const Program>();
			return nil;
		}

		ProgramPtr MakeLeaf(ProgramKind kind) {
			auto program = std::make_shared<Program>();
			program->kind = kind;
			return program;
		}

		bool Equal(const ProgramPtr& lhs, const ProgramPtr& rhs) {
			if (lhs == rhs) {
				return true;
			}
			if (lhs == nullptr || rhs == nullptr || lhs->kind != rhs->kind) {
				return false;
			}
			return lhs->action == rhs->action
				&& lhs->formula == rhs->formula
				&& lhs->variables == rhs->variables
				&& Equal(lhs->left, rhs->left)
				&& Equal(lhs->right, rhs->right);
		}

		ProgramPtr MakeAction(CompoundAction action) {
			auto program = std::make_shared<Program>();
			program->kind = ProgramKind::Action;
			program->action = std::move(action);
			return program;
		}

		ProgramPtr MakeCheck(Formula formula) {
			auto program = std::make_shared<Program>();
			program->kind = ProgramKind::Check;
			program->formula = std::move(formula);
			return program;
		}

		ProgramPtr MakeBinary(ProgramKind kind, ProgramPtr left, ProgramPtr right) {
			auto program = std::make_shared<Program>();
			program->kind = kind;
			program->left = std::move(left);
			program->right = std::move(right);
			return program;
		}

		ProgramPtr MakeSequence(ProgramPtr left, ProgramPtr right) {
			if (left->kind == ProgramKind::Nil) {
				return right;
			}
			if (right->kind == ProgramKind::Nil) {
				return left;
			}
			return MakeBinary(ProgramKind::Sequence, std::move(left), std::move(right));
		}

		ProgramPtr MakeBranch(ProgramPtr left, ProgramPtr right) {
			return MakeBinary(ProgramKind::Branch, std::move(left), std::move(right));
		}

		ProgramPtr MakePick(std::vector<Variable> variables, ProgramPtr body) {
			auto program = std::make_shared<Program>();
			program->kind = ProgramKind::Pick;
			program->variables = std::move(variables);
			program->left = std::move(body);
			return program;
		}

		ProgramPtr MakeIteration(ProgramPtr body) {
			auto program = std::make_shared<Program>();
			program->kind = ProgramKind::Iteration;
			program->left = std::move(body);
			return program;
		}

		ProgramPtr MakeInterleaved(ProgramPtr left, ProgramPtr right) {
			if (left->kind == ProgramKind::Nil) {
				return right;
			}
			if (right->kind == ProgramKind::Nil) {
				return left;
			}
			return MakeBinary(ProgramKind::Interleaved, std::move(left), std::move(right));
		}

		ProgramPtr MakeSimultaneous(ProgramPtr left, ProgramPtr right) {
			if (left->kind == ProgramKind::Nil && right->kind == ProgramKind::Nil) {
				return MakeNil();
			}
			return MakeBinary(ProgramKind::Simultaneous, std::move(left), std::move(right));
		}

		class Lowerer {
		public:
			ProgramPtr Lower(const IProgram& source) {
				VariableRenaming renaming;
				auto result = Lower(source, renaming);
				if (!unbound_.empty()) {
					std::vector<std::string> names;
					names.reserve(unbound_.size());
					for (const auto& variable : unbound_) {
						names.emplace_back(variable.name());
					}
					std::ranges::sort(names);
					std::string message = "Program contains variables outside an explicit Pick: ";
					for (size_t i = 0; i < names.size(); ++i) {
						message += names[i];
						if (i + 1 != names.size()) {
							message += ", ";
						}
					}
					throw std::invalid_argument(message);
				}
				return result;
			}

			const VariableSet& PickVariables() const {
				return pick_variables_;
			}

		private:
			ProgramPtr Lower(const IProgram& source, const VariableRenaming& renaming) {
				if (dynamic_cast<const Nil*>(&source) != nullptr) {
					return MakeNil();
				}
				if (const auto* action = dynamic_cast<const ActionProgram*>(&source)) {
					RecordUnbound(FreeVariables(action->act), renaming);
					return MakeAction(RenameVariables(action->act, renaming));
				}
				if (const auto* check = dynamic_cast<const Check*>(&source)) {
					RecordUnbound(FreeVariables(check->holds), renaming);
					return MakeCheck(RenameFreeVariables(check->holds, renaming));
				}
				if (const auto* sequence = dynamic_cast<const Sequence*>(&source)) {
					return MakeSequence(Lower(*sequence->p, renaming), Lower(*sequence->q, renaming));
				}
				if (const auto* branch = dynamic_cast<const Branch*>(&source)) {
					return MakeBranch(Lower(*branch->p, renaming), Lower(*branch->q, renaming));
				}
				if (const auto* pick = dynamic_cast<const Pick*>(&source)) {
					VariableRenaming local = renaming;
					std::vector<Variable> variables;
					std::unordered_set<Variable> seen;
					for (const auto& variable : pick->args) {
						if (!seen.emplace(variable).second) {
							throw std::invalid_argument("Pick binds variable '" + variable.name() + "' more than once");
						}
						Variable renamed{"__scs_pick_" + std::to_string(next_pick_++) + "_" + variable.name()};
						local.insert_or_assign(variable, renamed);
						variables.emplace_back(renamed);
						pick_variables_.emplace(renamed);
					}
					return MakePick(std::move(variables), Lower(*pick->p, local));
				}
				if (const auto* iteration = dynamic_cast<const Iteration*>(&source)) {
					return MakeIteration(Lower(*iteration->p, renaming));
				}
				if (const auto* loop = dynamic_cast<const Loop*>(&source)) {
					return MakeIteration(Lower(*loop->p, renaming));
				}
				if (const auto* interleaved = dynamic_cast<const Interleaved*>(&source)) {
					return MakeInterleaved(Lower(*interleaved->p, renaming), Lower(*interleaved->q, renaming));
				}
				if (const auto* simultaneous = dynamic_cast<const Simultaneous*>(&source)) {
					return MakeSimultaneous(Lower(*simultaneous->p, renaming), Lower(*simultaneous->q, renaming));
				}
				if (const auto* conditional = dynamic_cast<const CgIf*>(&source)) {
					RecordUnbound(FreeVariables(conditional->check), renaming);
					const auto condition = RenameFreeVariables(conditional->check, renaming);
					return MakeBranch(
						MakeSequence(MakeCheck(condition), Lower(*conditional->p, renaming)),
						MakeSequence(MakeCheck(Negate(condition)), Lower(*conditional->q, renaming)));
				}
				if (const auto* loop = dynamic_cast<const CgWhile*>(&source)) {
					RecordUnbound(FreeVariables(loop->condition), renaming);
					const auto condition = RenameFreeVariables(loop->condition, renaming);
					return MakeSequence(
						MakeIteration(MakeSequence(MakeCheck(condition), Lower(*loop->do_prog, renaming))),
						MakeCheck(Negate(condition)));
				}
				throw std::invalid_argument("Unsupported IProgram subclass in characteristic-graph compiler");
			}

			void RecordUnbound(const VariableSet& variables, const VariableRenaming& renaming) {
				for (const auto& variable : variables) {
					if (!renaming.contains(variable)) {
						unbound_.emplace(variable);
					}
				}
			}

			size_t next_pick_ = 0;
			VariableSet pick_variables_;
			VariableSet unbound_;
		};

		Formula Final(const ProgramPtr& program) {
			switch (program->kind) {
			case ProgramKind::Nil:
				return true;
			case ProgramKind::Action:
				return false;
			case ProgramKind::Check:
				return program->formula;
			case ProgramKind::Sequence:
				return Conjoin(Final(program->left), Final(program->right));
			case ProgramKind::Branch:
				return Disjoin(Final(program->left), Final(program->right));
			case ProgramKind::Pick:
				return ExistentiallyQuantify(program->variables, Final(program->left));
			case ProgramKind::Iteration:
				return true;
			case ProgramKind::Interleaved:
			case ProgramKind::Simultaneous:
				return Conjoin(Final(program->left), Final(program->right));
			}
			throw std::logic_error("Unknown program kind");
		}

		struct PotentialStep {
			CompoundAction action;
			std::vector<Variable> variables;
			Formula guard = true;
			ProgramPtr residual;
		};

		std::vector<PotentialStep> PotentialSteps(const ProgramPtr& program) {
			switch (program->kind) {
			case ProgramKind::Nil:
			case ProgramKind::Check:
				return {};
			case ProgramKind::Action:
				return {{program->action, {}, true, MakeNil()}};
			case ProgramKind::Sequence: {
				auto steps = PotentialSteps(program->left);
				for (auto& step : steps) {
					step.residual = MakeSequence(step.residual, program->right);
				}
				for (auto step : PotentialSteps(program->right)) {
					step.guard = Conjoin(Final(program->left), step.guard);
					steps.emplace_back(std::move(step));
				}
				return steps;
			}
			case ProgramKind::Branch: {
				auto steps = PotentialSteps(program->left);
				auto right = PotentialSteps(program->right);
				steps.insert(steps.end(), std::make_move_iterator(right.begin()), std::make_move_iterator(right.end()));
				return steps;
			}
			case ProgramKind::Pick: {
				auto steps = PotentialSteps(program->left);
				for (auto& step : steps) {
					step.variables.insert(step.variables.begin(), program->variables.begin(), program->variables.end());
				}
				return steps;
			}
			case ProgramKind::Iteration: {
				auto steps = PotentialSteps(program->left);
				for (auto& step : steps) {
					step.residual = MakeSequence(step.residual, program);
				}
				return steps;
			}
			case ProgramKind::Interleaved: {
				auto steps = PotentialSteps(program->left);
				for (auto& step : steps) {
					step.residual = MakeInterleaved(step.residual, program->right);
				}
				auto right = PotentialSteps(program->right);
				for (auto& step : right) {
					step.residual = MakeInterleaved(program->left, step.residual);
				}
				steps.insert(steps.end(), std::make_move_iterator(right.begin()), std::make_move_iterator(right.end()));
				return steps;
			}
			case ProgramKind::Simultaneous: {
				std::vector<PotentialStep> steps;
				const auto left = PotentialSteps(program->left);
				const auto right = PotentialSteps(program->right);
				for (const auto& lhs : left) {
					for (const auto& rhs : right) {
						PotentialStep combined;
						for (const auto& action : lhs.action.Actions()) {
							combined.action.AppendAction(action);
						}
						for (const auto& action : rhs.action.Actions()) {
							combined.action.AppendAction(action);
						}
						combined.variables = lhs.variables;
						combined.variables.insert(combined.variables.end(), rhs.variables.begin(), rhs.variables.end());
						combined.guard = Conjoin(lhs.guard, rhs.guard);
						combined.residual = MakeSimultaneous(lhs.residual, rhs.residual);
						steps.emplace_back(std::move(combined));
					}
				}
				return steps;
			}
			}
			throw std::logic_error("Unknown program kind");
		}

		void MergeProgramVariables(VariableSet& target, const VariableSet& source) {
			target.insert(source.begin(), source.end());
		}

		VariableSet FreeProgramVariables(const ProgramPtr& program) {
			switch (program->kind) {
			case ProgramKind::Nil:
				return {};
			case ProgramKind::Action:
				return FreeVariables(program->action);
			case ProgramKind::Check:
				return FreeVariables(program->formula);
			case ProgramKind::Sequence:
			case ProgramKind::Branch:
			case ProgramKind::Interleaved:
			case ProgramKind::Simultaneous: {
				auto variables = FreeProgramVariables(program->left);
				MergeProgramVariables(variables, FreeProgramVariables(program->right));
				return variables;
			}
			case ProgramKind::Pick: {
				auto variables = FreeProgramVariables(program->left);
				for (const auto& bound : program->variables) {
					variables.erase(bound);
				}
				return variables;
			}
			case ProgramKind::Iteration:
				return FreeProgramVariables(program->left);
			}
			throw std::logic_error("Unknown program kind");
		}

		std::vector<Variable> LiveVariables(const ProgramPtr& program, const VariableSet& pick_variables) {
			auto free = FreeProgramVariables(program);
			std::vector<Variable> live;
			for (const auto& variable : free) {
				if (pick_variables.contains(variable)) {
					live.emplace_back(variable);
				}
			}
			std::ranges::sort(live, {}, &Variable::name);
			return live;
		}

		class Compiler {
		public:
			Compiler(CharacteristicGraph& graph, VariableSet pick_variables)
				: graph_(graph), pick_variables_(std::move(pick_variables)) {}

			void Compile(ProgramPtr initial) {
				graph_.lts = {};
				programs_.emplace_back(std::move(initial));
				states_.emplace_back(MakeState(0, programs_.front()));
				graph_.lts.AddState(states_.front(), nightly::State<CgState, CgTransition>{});
				graph_.lts.set_initial_state(states_.front());

				for (size_t current = 0; current < programs_.size(); ++current) {
					for (auto& step : PotentialSteps(programs_[current])) {
						if (const auto* enabled = std::get_if<bool>(&step.guard); enabled != nullptr && !*enabled) {
							continue;
						}
						const size_t target = FindOrAdd(step.residual);
						CgTransition transition{step.action, step.variables, step.guard};
						transition.SetId(UUID{});
						bool exists = false;
						for (const auto& existing : graph_.lts.at(states_[current]).transitions()) {
							if (existing.label() == transition && existing.to() == states_[target]) {
								exists = true;
								break;
							}
						}
						if (!exists) {
							graph_.lts.AddTransition(states_[current], transition, states_[target]);
						}
					}
				}
			}

		private:
			CgState MakeState(size_t number, const ProgramPtr& program) const {
				return CgState{number, Final(program), LiveVariables(program, pick_variables_)};
			}

			size_t FindOrAdd(const ProgramPtr& program) {
				for (size_t i = 0; i < programs_.size(); ++i) {
					if (Equal(programs_[i], program)) {
						return i;
					}
				}
				const size_t number = programs_.size();
				programs_.emplace_back(program);
				states_.emplace_back(MakeState(number, program));
				graph_.lts.AddState(states_.back(), nightly::State<CgState, CgTransition>{});
				return number;
			}

			CharacteristicGraph& graph_;
			VariableSet pick_variables_;
			std::vector<ProgramPtr> programs_;
			std::vector<CgState> states_;
		};

	}

	void CompileCharacteristicGraph(const IProgram& program, CharacteristicGraph& graph) {
		Lowerer lowerer;
		auto lowered = lowerer.Lower(program);
		Compiler{graph, lowerer.PickVariables()}.Compile(std::move(lowered));
	}

}
