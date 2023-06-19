# Simulation of a Recipe

Following the recipe program, not its execution trace, as it doesn't add enough context regarding non-determinstic iteration vs a conditional, for example.

## Actions
- Case `LastOp` = convenience special marker that signifies last operation, i.e. controller is complete
- Case of simple action = one of the resources must perform the action. The rest of the resources should ideally continuously follow Nop or some other trace.
- Case of compound actions = like the above but Nop isn't enough in that multiple resources must do the action at same time.

In the case of compound and simple actions, traces must always fall through to some end-state where there's no more steps or a non-deterministic
hold state before passing the turn back to the Controller, i.e. it must be a final state, not awaiting more actions.

## Sequence
Must be able to do all without changing plan state.

## Non-determinstic branching
Must be able to do all, with deviations in the `Plan` correspondingly created.

## If
In this case, based on the current plan state, which due to non-determinism may mean reaching an If statement from multiple angles, just follows the 
condition and moves forward, i.e. following the relevant sub-execution.

## While

## Loop (non-determistic iteration)

## Pick

## Interleaved concurrency
Interleaved concurrency, $\delta_1 || \delta_2$$ does not require simultaenous execution, i.e. it can decide to do one action before the other.
Generally, simultaneous concurrency is needed.