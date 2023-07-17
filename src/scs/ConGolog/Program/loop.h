#pragma once

#include <queue>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	// Shorthand for either \delta* or while(True) do (p) 
	struct Loop : public IProgram {
		std::shared_ptr<IProgram> p;

		Loop(const IProgram& p)
			: p(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Loop>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		StateMeta& meta, CgTransition transition = CgTransition()) const override {
			// Add transitions, then starting from current states to end of newly added states (by transitions),
			// add loop back by rewriting the last transition. @Cleanup
			auto curr = tracker.CurrentStates();
			std::queue<size_t> states_queue(tracker.CurrentStates().begin(), tracker.CurrentStates().end());
			p->AddTransition(graph, counter, tracker, meta, transition);
			
			while (!states_queue.empty()) {
				auto top = states_queue.front();
				states_queue.pop();
				std::vector<size_t> rem;
				bool process = false;
				std::vector<nightly::Transition<scs::CgState, scs::CgTransition>> transition_changes;

				for (auto& transition : graph.lts.states().at(top).transitions()) {
					if (tracker.In(transition.to().n)) {
						transition_changes.push_back(transition);
						rem.emplace_back(transition.to().n);
						tracker.AppendUniqueState(top);
						process = true;
					} else {
						states_queue.emplace(transition.to().n);
					}
				}

				if (process) {
					for (const auto& el : rem) {
						// Delete states that were now rewritten from
						tracker.RemoveState(el);
						graph.lts.EraseShallow(el);
					}

					// Remove the transitions to states no longer
					auto& transitions = graph.lts.states().at(top).transitions();
					for (const auto& change : transition_changes) {
						auto position = std::find(transitions.begin(), transitions.end(), change);
						if (position != transitions.end()) {
							transitions.erase(position);
						}
					}

					// Loop back to current states
					for (const auto& transition : transition_changes) {
						for (const auto& loop_back_state : curr) {
							auto add = transition;
							add.to().n = loop_back_state;
							graph.lts.states().at(top).transitions().push_back(add);
						}
					}
				}


			}

		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Loop>" << *p << "*";
			os << "\n";
			return os;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const Loop& prog) {
		prog.Print(os);
		return os;
	}


}