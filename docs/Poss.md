# Poss
- Stored in unordered_map<string, Poss>
- If string is incorrect, invalid unordered map key error
- ```cpp
		// Poss(ship(x), s) ≡ At(x, ShipDock, s)
		Formula pre_ship_form = Predicate{ "At", {Variable{"x"}, Object{"ShipDock"}} };
		bat.pre["ship"] = { std::vector<Term>{Variable{"x"}}, pre_ship_form };
```
	- A Poss formula is composed of two elements, Firstly, the terms needed to perform the action. Secondly the actual Formula to evaluate true/false against.
	- For any terms that aren't objects, the variables will be substituted in from action instantiation terms, using the order of the terms provided (which corresponds to the names of the variables).

# Poss union
There are special mappings for concurrent Poss checks which can be found in `poss_union.h` with default already provided which is for the Hinge example.