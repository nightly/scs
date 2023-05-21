# Todo

- Global BAT: simple merging stuff
- Execution traces
	- Make an execution a variant
	- 
- Synthesis: simple visitor pattern to mimic the recipe + cost-based search, through following execution traces
	- Note that, \delta^* must be limited, else there may be infinite executions
	- An invisible, implicit ConGolog construct could be added at the end of the program, called 'End'. Such that \delta;End. It would allow you to easily track when the program is complete.


- SSAs
	- Probably needs more powerful action comparisons

# Performance
- Instead of `std:unordered_set<std::string`, consider using `tries`
- Replace `std::unordered_map` container for a more efficient implementation
