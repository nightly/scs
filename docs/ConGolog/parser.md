# ConGolog parser

The parser owns its source and throws `std::runtime_error` with line and column information. It never terminates the process on malformed input. `%` and `//` comments, ASCII and Unicode operators, underscores, apostrophes, integer constants, and decimal constants are accepted.

## File form

Each file contains an optional `#BAT` block followed by exactly one required `#Program` block. Duplicate directives, content before the first directive, and `#BAT` after `#Program` are errors. The BAT block is delegated to the situation-calculus parser.

```text
#BAT
objects r1, bit5
rigid Route(r1, r1) = false
poss Drill(x, bit, r) = true
#Program
(pick x. Drill(x, bit5, r1))*
```

Variables are introduced by `pick` or `π`. An unbound action term is an object constant and must have a rigid declaration in the BAT for an exact model. Formula quantifiers remain ordinary first-order binders. `identifier(x)` tests that an object is a renameable identifier rather than a rigid constant.

## Programs and precedence

Supported forms are `Nil`, primitive actions, formula tests `φ?`, sequence `;`, nondeterministic branch `|`, `pick`/`pi`/`π`, postfix iteration `*`, `loop`, interleaving `||`, synchronized concurrency `|||`, `if ... then ... else ... endif`, and `while ... do ... endwhile`.

From highest to lowest precedence: postfix iteration, sequence, synchronized concurrency, interleaved concurrency, and nondeterministic branch. Thus `A || B ||| C` means `A || (B ||| C)`. `||` is ordinary interleaving and `|||` is synchronized execution.
