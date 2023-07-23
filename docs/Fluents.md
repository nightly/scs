# Relational Fluent
- Stored with valuations
	- 0-arity and n-arity are handled separately
- If a valuation, that is a vector of Objects being looked up doesn't exist within the fluent, $False$ is returned
- `AddValuation` allows overriding, i.e. you can pass in a valuation that already exists
