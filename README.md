[![Actions Status](https://github.com/luk036/lmi-solver-cpp/workflows/MacOS/badge.svg)](https://github.com/luk036/lmi-solver-cpp/actions)
[![Actions Status](https://github.com/luk036/lmi-solver-cpp/workflows/Windows/badge.svg)](https://github.com/luk036/lmi-solver-cpp/actions)
[![Actions Status](https://github.com/luk036/lmi-solver-cpp/workflows/Ubuntu/badge.svg)](https://github.com/luk036/lmi-solver-cpp/actions)

# 🧮 LmiSolverCpp

Linear Matrix Inequality (LMI) feasibility solver for modern C++.

This library solves LMI feasibility problems of the form

```
F0 - sum(x_k * F_k) ≻ 0
```

using the Ellipsoid Method (via [EllAlgo](https://github.com/luk036/ellalgo-cpp)) with
cutting-plane oracles.

## ✨ Features

- C++20, header-only-friendly implementation
- Cutting-plane oracles for standard and perturbed LMI feasibility
- `LDLTMgr` for numerically stable LDLᵀ factorization
- Eigen-based matrix types (`Eigen::MatrixXd` by default)
- [Modern CMake practices](https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/)
- Integrated doctest test suite
- Continuous integration via [GitHub Actions](https://help.github.com/en/actions/)
- Reproducible dependency management via [CPM.cmake](https://github.com/TheLartians/CPM.cmake)

## Requirements

- C++20 compiler (GCC, Clang, MSVC)
- CMake 3.14+
- Eigen 3.4+ (fetched via CPM if not installed)
- EllAlgo (fetched via CPM)

## Usage

### Build and test

```bash
cmake -S. -B build
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest
```

### Run a single test (doctest)

```bash
./build/test/LmiSolverTests -tc="Test LMI"
```

### Use as a CMake dependency

```cmake
CPMAddPackage(
  NAME LmiSolver
  GIT_TAG v1.0.1
  GITHUB_REPOSITORY luk036/lmi-solver-cpp
  OPTIONS "INSTALL_ONLY YES"
)
```

## API

| Header | Purpose |
|--------|---------|
| `lmisolver/lmi_oracle.hpp` | Standard LMI feasibility oracle |
| `lmisolver/lmi0_oracle.hpp` | LMI feasibility with fixed F0 |
| `lmisolver/ldlt_mgr.hpp` | LDLᵀ factorization manager |

## Additional Tools

- **Sanitizers**: `-DUSE_SANITIZER=Address`
- **Static analyzers**: `-DUSE_STATIC_ANALYZER=clang-tidy`
- **Code coverage**: `-DENABLE_TEST_COVERAGE=1`

## License

MIT
