# Interpretations and object domains

Objects have an explicit `ObjectKind`: rigid constants are fixed by every renaming, while identifiers may be renamed. `Interpretation` is the exact API name for the structural situation representation containing sparse dynamic relational extensions. Equality and hashing ignore historical action sequences.

The finite backend quantifies over its explicit object carrier plus every rigid constant. The faithful backend represents an infinite identifier domain using bounded active states and source-compatible renaming witnesses. Anonymous representatives used to decide quantified formulas are local to an evaluation and never become fluent values.

The active-domain bound counts renameable identifiers in dynamic tuples, not rigid constants and not the number of different identifiers seen over an execution. Program live bindings and pending requests contribute separately to the abstraction support bound.
