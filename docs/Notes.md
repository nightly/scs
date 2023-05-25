# Notes

Implementation specific notes.

# Situation Calculus

## Relational Fluent
- Stored with valuations
	- 0-arity and n-arity are handled separately
- If a valuation, that is a vector of Objects being looked up doesn't exist within the fluent, $False$ is returned
- `AddValuation` allows overriding, i.e. you can pass in a valuation that already exists

## Poss
- Stored in unordered_map<string, Poss>
- If string is incorrect, invalid unordered map key error
- ```cpp
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula pre_ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		bat.pre["ship"] = { std::vector<Term>{Variable{"x"}}, pre_ship_form };
```
	- A Poss formula is composed of two elements, Firstly, the terms needed to perform the action. Secondly the actual Formula to evaluate true/false against.
	- For any terms that aren't objects, the variables will be substituted in from action instantiation terms, using the order of the terms provided (which corresponds to the names of the variables).

## Do & Successor state axioms
- Note that, $Poss(a, s)$ is not checked in the `Do` function we implement.
	- You check $Poss$ before deciding to follow an action route (so you can abandon quickly), meaning it would be a repeated check in practical implementations
- Each successor state axiom corresponds to one fluent
- $Do$ will not instantiate any new fluents although it could so it just provides an update


## Compound Actions
$Poss(ca, s)$ is handled by independently checking that the execution is valid, for all situations

$Do(ca, s)$ is handled by sequentially applying effects

## Global BAT

# ConGolog
