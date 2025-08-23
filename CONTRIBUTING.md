# Contributing

Thanks for considering a contribution!

## Getting started
- Install toolchain (CMake + Ninja + Clang/GCC + Python).
- Build:
  ```bash
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
  cmake --build build -j
  ctest --test-dir build --output-on-failure
  ```

## Style
- Run `pre-commit run --all-files` before pushing.
- C++ formatting enforced by `.clang-format` (LLVM-based).

## Tests & coverage
- Add/adjust unit tests under `tests/`.
- To collect coverage:
  ```bash
  cmake -S . -B build-cov -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
  cmake --build build-cov -j
  ctest --test-dir build-cov
  ```

## Benchmarks
- Build with `-DBUILD_BENCH=ON` and run `bench/bench_vec`.

## Fuzzing
- Requires Clang+libFuzzer: build with `-DBUILD_FUZZ=ON -DCMAKE_CXX_COMPILER=clang++`.
- Smoke run: `./build-fuzz/fuzz_vec -max_total_time=60`.

## ABI
- Use `scripts/abi_dump.sh` / `scripts/abi_diff.sh` to track ABI changes.
