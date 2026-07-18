# Situation Calculus Synthesis
![standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)
![ci.yml](https://github.com/nightly/scs/actions/workflows/ci.yml/badge.svg)
![GitHub](https://img.shields.io/github/license/nightly/scs)

This tool provides **controller synthesis**/orchestration of resource programs $\delta_i$ (where each program has a corresponding logical action theory) to realise a high-level target program (recipe). Recipes and resource programs are in the **ConGolog** programming language and compiled to **characteristic graphs** by this tool, where resource programs can be non-terminating/infinite, but recipe programs must be terminating. 

Logical action theories for resource programs are given by **situation calculus** basic action theories, $\mathcal{D_i}$. Situation calculus is a second-order logic with equality for reasoning about actions in AI with three disjoint sorts: *actions*, *objects*, and *situations*. **Preconditions** are specified for each action (and possibly for certain configurations of compound/concurrent actions), and **successor state axioms** encode causal laws of changes to fluents as a result of performing actions (dynamic predicates).

This tool implements two different possible solvers/search algorithms for controller synthesis, namely: **A\* search** (A*), and **Greedy Search** (GS).

## Build instructions
### Requirements
- [CMake](https://cmake.org/) (>=3.26)
- [Git](https://git-scm.com/) (for submodule cloning)
- A C++23 compiler
- [Ninja](https://ninja-build.org/) (required by the supplied CMake presets), or another
  CMake-supported build tool such as Make when configuring without a preset
- [GraphViz](https://graphviz.org/) for visualisation (`dot` must be in PATH)

On Debian/Ubuntu, Fedora, or Arch Linux, respectively, the development tools can
be installed with:

```sh
sudo apt install cmake ninja-build g++ git graphviz
sudo dnf install cmake ninja-build gcc-c++ git graphviz
sudo pacman -S cmake ninja gcc git graphviz
```

Package names and the available CMake version vary between distribution releases.
Before configuring, the tools used by the preset build can be checked with:

```sh
cmake --version
ninja --version
c++ --version
dot -V
```

### Cloning & updating
Clone the repository alongside its submodules (shallow submodule cloning is optional).

```sh
git clone --recurse-submodules --shallow-submodules https://github.com/nightly/scs
git submodule update --init --recursive
```

During development, to correctly pull any newly added Git submodules to your local repository (`git pull` alone doesn't suffice):

```sh
git submodule update --recursive
```

### CMake
The supplied Linux presets use Ninja. From the repository root, configure and build
a release version with:

```sh
cmake --preset linux-release
cmake --build out/build/linux-release --parallel "$(nproc)"
```

For a debug build followed by the complete test suite:

```sh
cmake --preset linux-debug
cmake --build out/build/linux-debug --parallel "$(nproc)"
ctest --test-dir out/build/linux-debug --parallel "$(nproc)" --output-on-failure
```

If Ninja is unavailable but Make is installed, configure without a preset and
select CMake's Unix Makefiles generator:

```sh
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
ctest --test-dir build --output-on-failure
```

On macOS use the `macos-debug` preset. Windows presets are `x64-debug` and
`x64-release`. The project can also be opened in an IDE with CMake support.

## Layout 
- `app`: contains the interactive CLI and unified paper-results applications.
- `bench`: contains all the project's benchmarks
- `data`: has some sample programs
- `docs`: contains some incomplete basic documentation/implementation notes
- `examples`: small example programs that can be ran/viewed
- `exports`: any directory marked exports gives any system/user-generated output
- `external`: 3rd party dependencies added as Git submodules
- `src/scs`: SCS library code
- `tests`: holds all tests & test data

## Running benchmarks with memory profiling
To run benchmarks with memory profiling, navigate to `bin/Benchmarks` directory, for a given benchmark executable's file (e.g. `bm.exe`) run the following:
`bm.exe --benchmark_format=json`

This is required because Google Benchmark's console output/VS runner doesn't report memory usage statistics, it is only given properly in the JSON format.

## Reproducing the paper results

The unified `scs_paper` application runs the complete experimental evaluation and writes tab-separated text files. Use a Release build for meaningful timings:

```sh
cmake --preset linux-release
cmake --build out/build/linux-release --target scs_paper --parallel "$(nproc)"
./bin/Release/scs_paper
```

By default, each run creates `exports/paper-results/<UTC timestamp>/` containing `grounding.tsv`, `astar.tsv`, `gbfs.tsv`, `phase_cost.tsv`, `phase_transitions.tsv`, `scaling.tsv`, and `run.tsv`. The complete suite is intentionally sequential and can take several hours; its three-resource A* attempt has a three-hour default timeout.

Available controls are:

```sh
./bin/Release/scs_paper --list
./bin/Release/scs_paper --suite tables --astar-timeout 5m
./bin/Release/scs_paper --suite limits --output-dir exports/my-paper-run
```

Suites are `all`, `tables`, `grounding`, `controllers`, `limits`, and `scaling`. An explicitly selected output directory must be empty. Timings use Google Benchmark CPU time; wall time is included as a diagnostic column. Each measured iteration starts from fresh experiment and solver state with random seed `2010`, so results may differ from the original MSVC/Ryzen measurements.

# Credits
- [Situation calculus for controller synthesis in manufacturing systems with first-order state representation](https://www.sciencedirect.com/science/article/abs/pii/S0004370221001491) 
- [Boost's](https://github.com/boostorg/boost):
    - [Container hash library](https://github.com/boostorg/container_hash)
    - flat_map, flat_set
- [Google Test](https://github.com/google/googletest) and [Google Benchmark](https://github.com/google/benchmark)
- [Planning and verification in the agent language Golog](https://publications.rwth-aachen.de/record/229059)
- [ankerl::unordered_dense::{map, set}](https://github.com/martinus/unordered_dense)
