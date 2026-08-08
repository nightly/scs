# `Do()` and successor-state axioms

`Situation::Do` assumes that the caller has already checked `Poss`; synthesis checks executability before progression so repeating it inside `Do` would be redundant.

Relational fluents are closed-world sparse relations. They store only true tuples, and every missing tuple is false. `AddValuation(tuple, false)` therefore removes the tuple instead of retaining an explicit false row.

For a ground action or compound action, progression builds a finite relevant object support from the objects declared by the BAT, objects in the current true fluent tuples, and objects named by the action. For every fluent whose successor-state axiom mentions the action, `Do` evaluates the axiom over the Cartesian power of that support at the fluent's arity and stores exactly the tuples that evaluate true. This permits a ground action to introduce an identifier that was not declared in `bat.objects`, while keeping each individual progression finite.

The successor-state axiom form is `F(vector<terms>, formula)`. The variables in `vector<terms>` are assigned from each candidate tuple before the formula is evaluated. Two variable names are reserved while evaluating an axiom:

- `a` is assigned the complete action or compound action being performed and supports action equality tests.
- `cv` is assigned the fluent's truth value in the source situation.

Quantifiers use the same finite relevant support during executability and progression. This is the finite-support backend; fully general infinite-domain equality types and fresh representative generation belong to the bounded-state abstraction layer.

## Compound actions

A successor-state axiom evaluates a compound action as one simultaneous action. It may test for multiple component actions and produce their combined effect, but progression never orders or applies the components sequentially.
