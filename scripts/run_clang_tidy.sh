#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null
fi

SDK_PATH=""
if command -v xcrun >/dev/null 2>&1; then
  SDK_PATH="$(xcrun --show-sdk-path 2>/dev/null || true)"
fi

FILES=()
if [[ "$#" -gt 0 ]]; then
  for to in "$@"; do
    FILES+=("$to")
  done
else
  while IFS= read -r to; do
    FILES+=("$to")
  done < <(
    rg --files "$ROOT_DIR/include" "$ROOT_DIR/src" "$ROOT_DIR/tests" -g '*.h' -g '*.hpp' -g '*.cpp' |
      rg -v '/include/httplib\.h$|/include/nlohmann/json\.hpp$'
  )
fi

if [[ "${#FILES[@]}" -eq 0 ]]; then
  echo "No files to lint."
  exit 0
fi

EXTRA_ARGS=()
if [[ -n "$SDK_PATH" ]]; then
  EXTRA_ARGS+=(--extra-arg=-isysroot --extra-arg="$SDK_PATH")
fi

clang-tidy "${FILES[@]}" -p "$BUILD_DIR" "${EXTRA_ARGS[@]}"
