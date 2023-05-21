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

## Do & Successor state axioms
- Note that, $Poss(a, s)$ is not checked in the `Do` function we implement.
	- You check $Poss$ before deciding to follow an action route (so you can abandon quickly), meaning it would be a repeated check in practical implementations
- Each successor state axiom corresponds to one fluent


## Compound Actions
$Poss(ca, s)$ is handled by independently checking that the execution is valid, for all situations

$Do(ca, s)$ is handled by sequentially applying effects

## Global BAT

# ConGolog
