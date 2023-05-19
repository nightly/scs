# Todo

- Global BAT: simple merging stuff
- Synthesis: simple visitor pattern to mimic the recipe + cost-based search, through following execution traces
	- Note that, \delta^* must be limited, else there may be infinite executions
	- An invisible, implicit ConGolog construct could be added at the end of the program, called 'End'. Such that \delta;End. It would allow you to easily track when the program is complete.
- Execution traces
	- Top-level array
	- 

# Cleanups
- Standardise Poss and Successor State Axiom, are they both data classes or performing some computations?
