# Recursive Descent Situation Calculus Parser

The situation calculus parser lives in `src/scs/SituationCalculus/Parser/` and builds the existing runtime objects directly: `Formula`, `Poss`, `Successor`, `Situation`, and `BasicActionTheory`.

## Public API

- `Formula ParseScFormula(std::string_view, const ParseContext& = {})`
- `BasicActionTheory ParseBasicActionTheory(std::string_view, ScParseOptions = {})`
- `void ValidateBasicActionTheory(const BasicActionTheory&, ValidationMode)`
- `SituationCalculusParser` exposes the same parsing operations plus `Diagnostics()` for local warnings.

Parser errors throw `std::runtime_error` with line and column information. Local BAT validation records warnings; global validation throws when cross-resource requirements are not satisfied.

## BAT Syntax

BAT statements are newline-separated. Blank lines are allowed, and comments may start with `#`, `%`, or `//`.

```text
objects brass, tube, 5
type Load = manufacturing
init at(brass, 2) = false
poss Load(part, i) = part(part) and on_site(part)
ssa at(part, i) = a = In(part, i) or a = Load(part, i) or (cv and a != Out(part, i))
```

Supported action types are `manufacturing`, `nop`, `transfer`, and `preparatory`. The old misspelling `prepatory` is intentionally rejected.

## Formula Syntax

Readable ASCII is canonical:

```text
forall x,y. ready(x) and x != y
exists i. at(part, i)
not (a = Release(part, i))
```

Aliases and precedence:

| Operation | Accepted syntax | Precedence |
| --- | --- | --- |
| Universal quantifier | `forall`, `∀` | prefix |
| Existential quantifier | `exists`, `∃` | prefix |
| Not | `not`, `¬`, `!` | 6 |
| Equal | `=`, `==` | 5 |
| Not equal | `!=`, `≠` | 5 |
| And | `and`, `∧`, `&&`, `^` | 4 |
| Or | `or`, `∨`, `||` | 3 |
| Implies | `implies`, `⊃`, `->` | 2, right-associative |
| Equivalent | `equiv`, `≡` | 1, right-associative |

Quantifiers require a dot after their variable list: `forall x.`, `forall x,y.`, `∀x.`, or `∀ x,y.`.

## Symbol Resolution

- Formal Poss/SSA parameters and quantified identifiers resolve to `Variable`.
- Declared objects resolve to `Object`.
- Object declarations may be numeric, as in `objects 1, 2, 3`, but formal and quantified variable names cannot start with a digit.
- Formal and quantified variables cannot shadow declared objects, and cannot use reserved names.
- Calls in `a = Action(...)` and `a != Action(...)` resolve to `Action`.
- Other calls resolve to `Predicate`.
- Explicit disambiguators are available: `obj(name)`, `var(name)`, `act(Name(...))`, and `pred(name(...))`.
- `coop(i, j)` resolves to `CoopPredicate`; `route(i, j)` resolves to `RoutePredicate`.

### Reserved Names

| Name | Meaning | Where it is used |
| --- | --- | --- |
| `a` | The action being evaluated by a successor-state axiom. SSA formulas compare it to concrete action terms, such as `a = Load(part, i)` or `a != Out(part, i)`. | `ssa` formulas |
| `cv` | The current value of the SSA target fluent before action `a` is applied. It is used to express persistence/frame behavior, such as `cv and a != Out(part, i)`. | `ssa` formulas |

## Validation

Local parsing permits cross-resource references and records warnings. Global validation is run by `CombineBATs` after resources are merged and fails if any referenced fluent or SSA target has no initial valuation in the merged BAT.

Validation also checks:

- Known fluent/action arities when enough information is available.
- Object constants used in formulas or action terms are in the merged BAT object domain.
- Duplicate object declarations, duplicate formal parameters, duplicate quantified variables, invalid variable names, and reserved-name collisions.

Initial fluent valuations are allowed to mention resource-local constants before the full resource set is merged. This keeps shared/common BAT fragments usable with different resource subsets while still validating object constants that appear in executable formulas.
