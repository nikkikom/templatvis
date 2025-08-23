#!/usr/bin/env bash
set -euo pipefail
# Compare ABI between two build directories (default: build vs build-prev)
a=${1:-build}
b=${2:-build-prev}
a_dir="abi/${a}"
b_dir="abi/${b}"

if [[ ! -d "$a_dir" || ! -d "$b_dir" ]]; then
  echo "Missing ABI dumps: $a_dir or $b_dir"; exit 1
fi

status=0
for a_xml in "$a_dir"/*.abi.xml; do
  base=$(basename "$a_xml")
  b_xml="${b_dir}/${base}"
  if [[ ! -f "$b_xml" ]]; then
    echo "WARNING: counterpart not found for ${base} in ${b_dir}"
    continue
  fi
  echo "ABIDIFF: ${base}"
  # Fail on incompatible changes; ignore harmless changes with --leaf-changes-only if desired
  abidiff "$b_xml" "$a_xml" || status=1
done

exit $status
