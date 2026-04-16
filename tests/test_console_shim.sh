#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
main_c="${repo_root}/src/main.c"

# Regression guard: compatibility shims must map the new 4.5+ names
# to the legacy names, never the other way around.
rg -q '^#define consoleSetTextGfxPtr consoleSetTextVramBGAdr$' "${main_c}"
rg -q '^#define consoleSetTextMapPtr consoleSetTextVramAdr$' "${main_c}"

if rg -q '^#define consoleSetTextVramBGAdr consoleSetTextGfxPtr$' "${main_c}"; then
  echo "Backward shim mapping detected for BG text API" >&2
  exit 1
fi

if rg -q '^#define consoleSetTextVramAdr consoleSetTextMapPtr$' "${main_c}"; then
  echo "Backward shim mapping detected for map text API" >&2
  exit 1
fi

echo "test_console_shim: ok"
