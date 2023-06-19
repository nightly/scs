# Traces

## Action
- Added to the current trace, primitive type

## Check/conditional
- Added to the current trace, primitive type

## Sequence
- Adds execution $e_1$ and $e_2$

Example: $a_1; (a_2 | a_3 | a_4); a_5$

## Branch
- 

## While
- Branch
	- Condition, then non-deterministic loop execution ($\delta^*$)
	- $\neg \: condition$ and follows the rest of the execution
- Outwards the next statement is $\neg \: condition$ to exit the loop follow by the rest of the program, if the exit condition isn't met, remains in loop indefinitely (can use fairness assumption).

## Interleaved concurrency (||)
- Two potential traces exist, i.e. doing $e_1$ or $e_2$ first, as well as switching between the two.

## Simultaneous concurrency (|||)
- Not supported with traces as it is only for recipes
