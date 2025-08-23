#!/usr/bin/env bash
set -euo pipefail
THRESHOLD_KB=${THRESHOLD_KB:-4096}
for so in libdemo_template_export libthrowers_template_export libdemo_inst_export libthrowers_inst_export libdemo_all_export libthrowers_all_export; do
  for ext in .so .dylib .dll; do
    file="build/${so}${ext}"
    if [[ -f "$file" ]]; then
      size_kb=$(du -k "$file" | cut -f1)
      echo "$file: ${size_kb} KB"
      if (( size_kb > THRESHOLD_KB )); then
        echo "ERROR: $file exceeds threshold ${THRESHOLD_KB} KB"
        exit 1
      fi
    fi
  done
done
