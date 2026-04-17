#!/usr/bin/env sh
set -eu

# Regression guard: init_video must bring screen brightness up from reset.
if ! rg -n "setBrightness\\(0x0F\\)" src/main.c >/dev/null; then
  echo "Missing setBrightness(0x0F) in init_video; launch can remain black." >&2
  exit 1
fi

# Keep the startup sequence explicit: brightness should be set before screen-on.
brightness_line="$(rg -n "setBrightness\\(0x0F\\)" src/main.c | cut -d: -f1 | head -n1)"
screen_on_line="$(rg -n "setScreenOn\\(\\)" src/main.c | cut -d: -f1 | head -n1)"

if [ -z "${brightness_line}" ] || [ -z "${screen_on_line}" ] || [ "${brightness_line}" -ge "${screen_on_line}" ]; then
  echo "Expected setBrightness(0x0F) before setScreenOn()." >&2
  exit 1
fi

echo "test_boot_brightness_regression: ok"
