# Compatible facility composition

`ComposeFacility` replaces the old unqualified BAT union. `RigidDatabase` preserves finite explicit true and false facts; the same declaration representation is retained for initial dynamic facts during composition. Rigid constants, rigid relations, and initial facts merge only when arities and explicit truths are consistent. Local action schemas remain associated with their `ResourceIndex`, and action occurrences inside local SSAs are indexed automatically.

Every dynamic fluent must have exactly one final SSA. Duplicate SSAs for a shared fluent are rejected unless `FacilityComposition::combined_successors` supplies the explicit facility-wide axiom. Routing and cooperation are ordinary finite rigid relations rather than special matrices.

The facility also owns typed callbacks for joint executability, observation, and positive cost. Observation returns `std::optional<CompoundAction>`; `std::nullopt` denotes the internal label `τ`.
