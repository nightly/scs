# Observation and request matching

Legacy prefix unification is not used by exact synthesis. Environment first chooses a fully ground recipe request. Controller may take internal `τ` facility transitions, but a visible facility transition is legal only when the facility observation is structurally equal to that pending request, including action names, arities, rigid constants, and identifier equality patterns.

In a concrete `ControllerSession`, representative identifiers are matched bijectively to concrete request identifiers. Rigid constants must match themselves. Environment-created identifiers retained only in recipe memory are supplied in `RecipeEdgeChoice::bindings`; the fresh-identifier provider is used only for Controller-created identifiers. The selected representative facility label is inverted through the stored edge witness, and target-only Controller identifiers must not collide with any identifier used earlier in the session.
