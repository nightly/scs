# Progression and successor-state axioms

`Situation` is the implementation's structural, hashable interpretation. Its identity contains sorted sparse dynamic-fluent extensions and does not contain action history. Missing tuples are false under the closed-world assumption.

`Situation::Do` assumes executability has already been checked. For every dynamic fluent it enumerates candidate tuples only over `adom(I) ∪ ids(action) ∪ rigid`, evaluates the fluent's SSA against the unchanged source interpretation, and installs all resulting extensions simultaneously. A ground action may therefore introduce a fresh identifier without requiring an eager global object universe. Evaluator-local anonymous quantifier representatives are never members of this candidate carrier and cannot leak into persisted tuples.

The SSA variables `a` and `cv` are reserved. `a` contains the complete resource-indexed facility action encoded as an indexed compound action, while `cv` is the source truth value of the candidate fluent tuple. `ResourceActionOccurs` and `JointActionMatches` construct resource-aware action-event formulas; resource positions and repeated `Nop` actions are preserved.

Finite overloads quantify over the supplied carrier plus rigid constants. The default exact progression path uses infinite generic semantics: it adds enough anonymous identifiers to represent every syntactically relevant equality type while retaining a finite evaluation. It also probes target equality types outside the candidate carrier and rejects an SSA whose successor extension would be infinite, because such a state has no finite sparse representation.
