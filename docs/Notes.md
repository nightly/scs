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

## Do

## Successor state axioms

## Compound Actions
$Poss(ca, s)$ is handled by independently checking that the execution is valid, for all situations

$Do(ca, s)$ is handled by sequentially applying effects

## Global BAT

# ConGolog
