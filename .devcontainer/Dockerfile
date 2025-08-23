# syntax=docker/dockerfile:1
FROM mcr.microsoft.com/devcontainers/base:ubuntu

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends     build-essential cmake ninja-build clang lld gdb     python3 python3-pip python3-venv     git curl ca-certificates     doxygen graphviz     valgrind     libdw-dev libabigail-dev abigail-tools     lcov   && rm -rf /var/lib/apt/lists/*

# pre-commit & clang-format helper (clang-format from LLVM apt could be installed, but cpptools ext also provides format)
RUN pip3 install --no-cache-dir pre-commit

# Create user vscode if not present (devcontainers base already has it)
