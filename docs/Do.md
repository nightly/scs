# Do() and Successor State Axioms (SSAs)
- Note that, $Poss(a, s)$ is not checked in the `Do` function we implement.
	- You check $Poss$ before deciding to follow an action route (so you can abandon quickly), meaning it would be a repeated check in practical implementations
- Each successor state axiom corresponds to one fluent
- $Do$ will not instantiate any new fluents although it could, it just provides an update of those that already exist. `DCA`.

- The SSA follows the form of `F(vector<terms>, formula)`
	- So, when looping over fluents that involve a particular successor, substitutes in the variable names from F with the current fluents value
	- This then powers the action term equality check/unification
		- **Each fluent instantiation must decide if it's impacted by a particular action or non-action basically**

- Reserved keywords are as follows:
	- `a` = used for equality checks, i.e. it is substituted in with the exact action term that can be unified based on the subtituted in variables to see equality.
	- `cv` = current value, which at $Do$ evaluation time will be substituted in with the boolean of the current fluent value. 
	Used in a disjunction chain of the positive effects.

# Compound Actions
A successor state axiom can reason between simple actions contained within compound actions. This also means that a successor state axiom can make a statement about multiple simple actions that occured in parallel (all within the same compound action), for special facility mappings.

