# Parser

A parser for ConGolog + First Order Logic.

# Precedence and Associativity

## First Order Logic

| Operator  | Precedence | Associativity |
|-----------|------------|---------------|
| $\neg$    |     1      |      -        |
| $\land$   |     2      |      -        |
| $\lor$    |     3      |      -        |
| $\to$     |     4      |     Right     |
| $\equiv$  |     5      |     Right     |
| $\exists$ |     6      |      -        |
| $\forall$ |     6      |      -        |

## ConGolog

| Operator               |    Name                       | Precedence | Associativity |
|------------------------|-------------------------------|------------|---------------|
| *                      |  Non-deterministic iteration  |     1      |     Right     |
| $\delta_1;\delta_2     |  Sequence                     |     1      |     Left      |
| $\delta_1|\delta_2     |  Non-determinism              |     1      |     Left      |
| $\delta_1||\delta_2    |  Interleaved concurrency      |     1      |     Left      |
| $\delta_1|||\delta_2   |  Synchronized concurrency     |     1      |     Left      |

# Grammar
EBNF notation.

`ConGolog file .cgl`
```
agent ::= directive {statement} directive {statement}
directive ::= "#BAT" | "#Program"
```

Note that directive BAT must come first currently to parse action and fluent symbols.

`FOL`
```
equality ::= term "=" term
term ::= object | variable
```
