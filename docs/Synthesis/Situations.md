# Situations
We construct a global BAT D and a global situation.

Instead of retaining individual situations for resources, we use a global situation, with the following in mind: 
- Fluents can be indexed with a variable. So $at(part)$ becomes $at(part, i)$.
- This global situation has an action history of compound actions, not simple actions. 
	- Compound actions implicitly are indexed by resource, i.e. first resource will be represented by simple action 0 of the compound action.
- Successor state axioms and preconditions should have the indexes involved as last argument.

The A* and greedy-search synthesis flows use Markovian situations by default. A Markovian situation retains only the current fluent valuation: action executability and deterministic successor states are cached by that valuation, and the action-history vector is not copied into subsequent stages. Pass `markovian_situations = false` to the solver or example runner to retain the complete action history and bypass the fluent-state cache.

Execution programs and characteristic graphs are kept essentially as they are:
- We just write the constant for each program in the choosable actions.
	- Without doing so there is no way to map between fluents from various resources, at(part, **i**).
- This could be automatically done ('rewritten') for us (same as $at(part, i)$) but explicitly allows more expressivity by the usage of constants and not just $i$ as a variable.
