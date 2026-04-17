#!/usr/bin/env bash
set -euo pipefail

main_c="src/main.c"

# Enforce the PVSnesLib 4.5.0 text-console API path.
rg -n "consoleSetTextGfxPtr\(" "${main_c}" >/dev/null
rg -n "consoleSetTextMapPtr\(" "${main_c}" >/dev/null

# Guard against legacy/incorrect alias APIs that caused breakage.
if rg -n "consoleSetTextVramBGAdr\(|consoleSetTextVramAdr\(" "${main_c}" >/dev/null; then
  echo "Unexpected legacy console text VRAM API call found"
  exit 1
fi

# Sprite and input calls expected in 4.5.0 headers.
rg -n "oamInitGfxSet\(" "${main_c}" >/dev/null
rg -n "oamSet\(" "${main_c}" >/dev/null
rg -n "padsCurrent\(" "${main_c}" >/dev/null

# Background API names used by 4.5.0.
rg -n "bgSetGfxPtr\(" "${main_c}" >/dev/null
rg -n "bgSetMapPtr\(" "${main_c}" >/dev/null
rg -n "bgSetDisable\(" "${main_c}" >/dev/null

echo "test_pvsneslib_450_api_regression: ok"
