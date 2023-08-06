# Situation Calculus Synthesis
![standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)
![ci.yml](https://github.com/nightly/scs/actions/workflows/ci.yml/badge.svg)
![GitHub](https://img.shields.io/github/license/nightly/scs)

This tool provides **controller synthesis**/orchestration of resource programs $\delta_i$ (where each program has a corresponding logical action theory) to realise a high-level target program (recipe). Recipes and resource programs are in the **ConGolog** programming language and compiled to **characteristic graphs** by this tool, where resource programs can be non-terminating/infinite, but recipe programs must be terminating. 

Logical action theories for resource programs are given by **situation calculus** basic action theories, $\mathcal{D_i}$. Situation calculus is a second-order logic with equality for reasoning about actions in AI with three disjoint sorts: *actions*, *objects*, and *situations*. **Preconditions** are specified for each action (and possibly for certain configurations of compound/concurrent actions), and **successor state axioms** encode causal laws of changes to fluents as a result of performing actions (dynamic predicates).

This tool implements three different possible search algorithms for controller synthesis, namely: **A\* search**, **Greedy Best First Search** (GBFS), and **Simple Parallel A\*** (SPA*, shared frontier, experimental).

## Build instructions
### Requirements
- [CMake](https://cmake.org/) (>=3.26)
- [Git](https://git-scm.com/) (for submodule cloning)
- C++23 compiler (tested on Windows MSVC 19.36.32537.0)
- [GraphViz](https://graphviz.org/) for visualisation (`dot` must be in PATH)

### Cloning & updating
Clone the repository alongside its submodules (shallow submodule cloning is optional).
```
git clone --recurse-submodules --shallow-submodules https://github.com/nightly/scs
git submodule update --init --recursive
```

During development, to correctly pull any newly added Git submodules to your local repository (`git pull` alone doesn't suffice):
```
git submodule update --recursive
```

### CMake
Build the project using CMake or simply open it using an IDE that has CMake support.
```
cmake -S. -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

All of the tests can also be run with the following:
```
cd build
ctest --output-on-failure --verbose
```

## Layout 
- `app`: contains a CLI executable application.
- `bench`: contains all the project's benchmarks
- `data`: has some sample programs
- `docs`: contains some incomplete basic documentation/implementation notes
- `examples`: small example programs that can be ran/viewed
- `exports`: any directory marked exports gives any system/user-generated output
- `external`: 3rd party dependencies added as Git submodules
- `src/scs`: SCS library code
- `tests`: holds all tests & test data

# Credits
- [Situation calculus for controller synthesis in manufacturing systems with first-order state representation](https://www.sciencedirect.com/science/article/abs/pii/S0004370221001491). 
- [Boost container hash library](https://github.com/boostorg/container_hash)
- Boost flat_map, flat_set
- [Planning and verification in the agent language Golog](https://publications.rwth-aachen.de/record/229059)
- [ankerl::unordered_dense::{map, set}](https://github.com/martinus/unordered_dense)