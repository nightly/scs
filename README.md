# Situation Calculus Synthesis

![standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)
![ci.yml](https://github.com/nightly/scs-devs/actions/workflows/ci.yml/badge.svg)

## Build instructions
### Requirements
- [CMake](https://cmake.org/) (>=3.22)
- [Git](https://git-scm.com/) (for submodule cloning)
- C++20 compiler

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
- `benchmarks`: contains all the project's benchmarks
- `data`: has some sample programs
- `external`: 3rd party dependencies added as Git submodules
- `src/scs`: SCS library code
- `tests`: holds all tests & test data

# Credits
- [Situation calculus for controller synthesis in manufacturing systems with first-order state representation](https://www.sciencedirect.com/science/article/abs/pii/S0004370221001491). 
- PhDP, HazardyKnusperkeks
