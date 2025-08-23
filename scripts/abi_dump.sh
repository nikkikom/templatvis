#!/usr/bin/env bash
set -euo pipefail
# Dump ABI of built shared libraries into abi/<buildname>/
build_dir=${1:-build}
out_dir="abi/${build_dir}"
mkdir -p "${out_dir}"

shopt -s nullglob
libs=( "${build_dir}"/libdemo_*.so "${build_dir}"/libthrowers_*.so        "${build_dir}"/libdemo_*.dylib "${build_dir}"/libthrowers_*.dylib )
if (( ${#libs[@]} == 0 )); then
  echo "No shared libraries found in ${build_dir}"; exit 0
fi

for lib in "${libs[@]}"; do
  base=$(basename "$lib")
  xml="${out_dir}/${base}.abi.xml"
  echo "ABIDUMP: $lib -> $xml"
  abidw --no-corpus-path --out-file "$xml" "$lib" || { echo "abidw failed for $lib"; exit 1; }
done

echo "ABI dumps written to ${out_dir}"
