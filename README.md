
[![CI](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml/badge.svg)](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml)
[![ASan](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml/badge.svg?event=push&job=asan-job)](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml)
[![UBSan](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml/badge.svg?event=push&job=ubsan-job)](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml)
[![TSan](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml/badge.svg?event=push&job=tsan-job)](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml)
[![Clang-18](https://img.shields.io/badge/clang-18-blue)](#)
[![Coverage](https://codecov.io/gh/nikkikom/templatvis/branch/master/graph/badge.svg)](https://codecov.io/gh/nikkikom/templatvis)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue.svg)](https://nikkikom.github.io/templatvis/)

# templatvis4 — шаблонная видимость (3 кейса)

[![CI](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml/badge.svg)](https://github.com/nikkikom/templatvis/actions/workflows/ci.yml)
[![Coverage](https://codecov.io/gh/nikkikom/templatvis/branch/main/graph/badge.svg)](https://codecov.io/gh/nikkikom/templatvis)
[![CodeQL](https://github.com/nikkikom/templatvis/actions/workflows/codeql.yml/badge.svg)](https://github.com/nikkikom/templatvis/actions/workflows/codeql.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue.svg)](https://nikkikom.github.io/templatvis/)

## Кейсы
- **template_export** — `DEMO_EXPORT` на определении шаблона (class).
- **inst_export** — `DEMO_EXPORT` на явной инстанциации.
- **all_export** — `DEMO_EXPORT` сразу везде (class + extern line + instantiation).

Деструктор можно переключать: inline (`-DDEMO_INLINE_DTOR=ON`) или out-of-line (по умолчанию).

## Сборка
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Документация
```bash
doxygen Doxyfile
# Pages workflow задеплоит на https://nikkikom.github.io/templatvis/
```

## Dev Container
В каталоге `.devcontainer/` есть конфиг для VS Code Dev Containers. Внутри контейнера уже стоят: gcc/clang, CMake, Ninja, Doxygen, Valgrind, libabigail, pre-commit.

Запуск в VS Code: `Reopen in Container`.

## Pre-commit
Установи и активируй хуки:
```bash
pip3 install pre-commit    # либо 'brew install pre-commit'
pre-commit install
pre-commit run --all-files
```
Форматирование управляется `.clang-format`; есть линтеры для CMake и базовые проверки.

## ABI-дампы и сравнение (libabigail)
Снять дампы ABI:
```bash
bash scripts/abi_dump.sh build
```
Сравнить ABI текущей сборки с предыдущей:
```bash
bash scripts/abi_dump.sh build-prev   # предварительно собери build-prev
bash scripts/abi_diff.sh build build-prev
```
В CI есть job `abi-compare`, который на PR сравнивает ABI с `main` и публикует дампы артефактом.

## Benchmarks
```bash
cmake -S . -B build-bench -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCH=ON
cmake --build build-bench -j
./build-bench/bench_vec --benchmark_min_time=0.1
```

## Fuzzing
```bash
cmake -S . -B build-fuzz -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_FUZZ=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz -j
./build-fuzz/fuzz_vec -max_total_time=30
```

## Детерминизм на macOS (LC_UUID)

На macOS компоновщик `ld64` по умолчанию добавляет в Mach-O бинарь команду `LC_UUID`
со случайным UUID. Это ломает воспроизводимость сборок: бинарники, собранные с одинаковыми исходниками,
будут иметь разные UUID.

- Удалять `LC_UUID` у **динамических библиотек (.dylib)** нельзя — линковщик откажется связывать такие зависимости.
- Поэтому в проекте введена опция:

```bash
-DAPPLE_NO_UUID_FOR_EXE=ON
```

Она убирает `LC_UUID` только у **исполняемых файлов** (`main_template_export`, `main_inst_export`, `main_all_export`),
оставляя UUID у библиотек.

По умолчанию (`OFF`) UUID сохраняются у всех артефактов.
