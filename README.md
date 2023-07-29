# Situation Calculus Synthesis

![standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)
![ci.yml](https://github.com/nightly/scs/actions/workflows/ci.yml/badge.svg)

## Build instructions
### Requirements
- [CMake](https://cmake.org/) (>=3.26)
- [Git](https://git-scm.com/) (for submodule cloning)
- C++23 compiler
- GraphViz for visualisation (`dot` in PATH)

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
- `external`: 3rd party dependencies added as Git submodules
- `src/scs`: SCS library code
- `tests`: holds all tests & test data

# Credits
- [Situation calculus for controller synthesis in manufacturing systems with first-order state representation](https://www.sciencedirect.com/science/article/abs/pii/S0004370221001491). 
- Boost container hash library
- Boost flat_map, flat_set
- [Planning and verification in the agent language Golog](https://publications.rwth-aachen.de/record/229059)
- PhDP, HazardyKnusperkeks
- ankerl::unordered_dense::{map, set}