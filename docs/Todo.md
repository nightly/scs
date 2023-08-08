# Todo
- While functional fluents do not add any further expressivity, would be a nice addition
- Complete implementation of Characteristic Graph
	- Some basic constructs that were not used during development like `while` loops are not supported even though it's easy to add.
	- Could it be written in a better way altogether?

# Improvements

## Cleanups
- Poss union improvements
	- Currently, the mappings vector requires you to provide a function pointer for a containment check and then the actual concurrent precondition check, which then also takes over for having to check the rest of the actions also
	- Instead it should work as follows:
		- The check should not be a function pointer, but an abstract compound action. Any variables mentioned must be constant, e.g. $\{ In(var(part), var(i)), Out(var(part), var(j)) \}$ means that the part must be automatically consistent without us needing to check it in our concurrent precondition check pointer. Obviously, it still should be fast, not checking variables before it's confirmed that all such action names even exist.
		- Then the concurrent precondition check is actually just a formula that can mention variables/constants from any of the actions it specified in the abstract compound action set. Not a function pointer, it has no reason to be a function really.

## Performance
- Instead of `std:unordered_set<std::string`, consider using `tries`
