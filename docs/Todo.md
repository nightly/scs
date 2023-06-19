# Todo
- Poss consider union for part-handling situation independent predicate
- $Do$ use CompoundAction equality instead of just Action, so that it's more efficient (fluents only need to be updated/checked once instead of x times)

# Performance
- Instead of `std:unordered_set<std::string`, consider using `tries`
- Replace `std::unordered_map` container for a more efficient implementation
