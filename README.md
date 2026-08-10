# Situation Calculus Synthesis

SCS is a C++23 controller-synthesis library and command-line tool for relational ConGolog resource and recipe models. It compiles programs into characteristic graphs, constructs either an explicit finite arena or a finite faithful abstraction of an infinite identifier domain, solves the recurrent request-response game exactly, optimizes the worst-case response cost, lifts the symbolic strategy to an executable finite-memory controller, and validates that controller by replaying the model independently.

## Semantics

The object domain is the disjoint union of renameable identifiers and rigid constants. Dynamic state is a sparse structural `Interpretation`: only true relational-fluent tuples are stored, and situation history is not part of state identity. A finite `RigidDatabase` retains explicit true and false relation facts so composition can reject inconsistent fragments. Finite-domain mode quantifies over `FiniteDomainBackend::explicit_objects` plus rigid constants. Faithful-abstraction mode preserves infinite-domain quantifier truth with evaluator-local anonymous representatives, progresses only tuples over the current active domain, action identifiers, and rigid constants, and canonicalizes bounded states modulo identifier renaming.

Facilities retain resource positions explicitly in `JointAction`; repeated local `Nop` actions and simultaneous operations are not collapsed. A facility supplies compatible resource BAT fragments plus deterministic, equivariant callbacks for joint executability, action observation (`std::nullopt` is `τ`), and positive transition cost. The only synthesis entry point is `Synthesise(problem, options)`.

The exact arena gives every enabled recipe edge and grounding to Environment. Controller responds with a finite sequence of internal facility actions followed by a visible action equal to the pending request. Final recipe locations offer `stop` without suppressing continuing recipe edges. The solver computes the recurrent winning region `νX. μY.(Goal ∪ PreE(X) ∪ PreC(Y))`, then searches for the least feasible response budget and extracts budget memory into the controller.

## Build and test

Requirements are CMake 3.26 or newer, Ninja for presets, a C++23 compiler, Git submodules, and Graphviz `dot` for rendered graph output.

```sh
git submodule update --init --recursive
cmake --preset linux-debug
cmake --build out/build/linux-debug --parallel "$(nproc)"
ctest --test-dir out/build/linux-debug --parallel "$(nproc)" --output-on-failure
```

Use `linux-release` for optimized builds, `macos-debug` on macOS, and `x64-debug` or `x64-release` on Windows. A non-preset build can use `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` followed by `cmake --build build`.

## Run the exact assembly example

The default CLI solves the parameterized three-resource assembly over an infinite stream of workpiece identifiers. The active-domain bound is two; the derived abstraction bounds are `B=7`, `k=5`, and pool size `19`; the default optimal worst-case response cost is `K*=10`.

```sh
./bin/Debug/scs_cli
./bin/Debug/scs_cli --worklist lower-cost-first
./bin/Debug/scs_cli --finite
./bin/Debug/scs_cli --export-prefix exports/assembly-controller
```

The export option writes both GraphViz (`.gv`) and TikZ (`.tex`) controller files.

## Paper artifact

`scs_paper` runs exact representation, worklist, validation, fresh-renaming, and phase-timing suites. It writes deterministic controller exports and `metrics.tsv`.

```sh
./bin/Release/scs_paper --suite all --output-dir exports/paper-exact-run
./bin/Debug/scs_paper --suite smoke --output-dir exports/paper-smoke
./bin/Debug/scs_paper --list
```

Suites are `all`, `smoke`, `finite`, `worklists`, and `validation`. An explicitly selected output directory must be empty.

## Layout

- `src/scs`: reusable relational semantics, ConGolog compiler, exact arena, solver, controller, validator, and exporters.
- `examples/Assembly`: programmatic and textual parameterized-assembly fixtures.
- `app`: exact CLI and paper artifact.
- `tests`: GoogleTest semantic, abstraction, solver, parser, lifting, and end-to-end tests.
- `docs`: syntax and semantic notes.

The supported theory fragment is relational, deterministic, complete-information, and Markovian. Sensing, nondeterministic effects, object-valued functions, and exogenous events are outside the current model.
