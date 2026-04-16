#!/usr/bin/env sh
set -eu

# Regression guard: do not reintroduce alias macros that remap
# consoleSetTextGfxPtr/consoleSetTextMapPtr to legacy names.
if rg -n "#define[[:space:]]+consoleSetTextGfxPtr|#define[[:space:]]+consoleSetTextMapPtr" src/main.c >/dev/null; then
  echo "Found forbidden consoleSetText* alias macro in src/main.c"
  exit 1
fi

# Ensure init_video still sets text VRAM pointers explicitly.
rg -n "consoleSetTextGfxPtr\(" src/main.c >/dev/null
rg -n "consoleSetTextMapPtr\(" src/main.c >/dev/null

echo "test_sprite_vram_alias_regression: ok"
