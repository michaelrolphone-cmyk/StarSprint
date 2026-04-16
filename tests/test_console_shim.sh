#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
main_c="${repo_root}/src/main.c"

# Regression guard: do not alias these APIs via preprocessor defines.
if rg -q '^#define consoleSetTextGfxPtr ' "${main_c}"; then
  echo "Unexpected alias for consoleSetTextGfxPtr" >&2
  exit 1
fi

if rg -q '^#define consoleSetTextMapPtr ' "${main_c}"; then
  echo "Unexpected alias for consoleSetTextMapPtr" >&2
  exit 1
fi

echo "test_console_shim: ok"
