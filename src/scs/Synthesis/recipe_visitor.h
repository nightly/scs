#pragma once

#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/execution.h"
#include "scs/Synthesis/plan.h"

namespace scs {

	struct RecipeVisitor {
	public:
		const IProgram* recipe_prog;
		bool is_complete = false;
		Plan* plan;
	public:
		RecipeVisitor(const IProgram* prog, Plan* plan) : recipe_prog(prog), plan(plan) {}

		void operator() (const Action& a) {
			if (a.name == "LastOp") {
				is_complete = true;
			} else {
				
			}
		}

		void operator() (const CompoundAction& ca) {

		}

		void operator()(IProgram* program) {
			if (auto ap = dynamic_cast<ActionProgram*>(program)) {

			} else if (auto seq = dynamic_cast<Sequence*>(program)) {

			} else if (auto nd = dynamic_cast<Branch*>(program)) {

			} else if (auto sim = dynamic_cast<Simultaneous*>(program)) {

			} else {
				SCS_CRITICAL("Unimplemented program type for recipe visitor");
				program->Print(std::cerr);
			}

		}

		void operator()(const Check& check) {
			// An independent conditional check (that is not part of other ConGolog constructs)

		}

		void operator()(const ActionProgram& ap) {
			if (ap.ca.IsSimple()) {
				this->operator()(ap.ca.Actions()[0]);
			} else {
				this->operator()(ap.ca);
			}
		}

		void operator()(const Sequence& seq) {

		}

		void operator()(const Branch& nd) {

		}

		void operator()(const Simultaneous& sim) {

		}

		void operator()(const CgIf& cg_if) {

		}

		void operator()(const CgWhile& cg_while) {

		}

		void operator()(const Pick& pick) {

		}

		void operator()(const Interleaved& interleaved) {

		}

		void operator()(const Iteration& iteration) {

		}

	};

}
