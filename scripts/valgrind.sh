#!/usr/bin/env bash
set -euo pipefail
if ! command -v valgrind >/dev/null; then
  echo "valgrind not installed"; exit 0
fi
for bin in main_template_export main_inst_export main_all_export; do
  if [[ -x "build/$bin" ]]; then
    valgrind --leak-check=full --error-exitcode=42 "build/$bin" || exit $?
  fi
done
