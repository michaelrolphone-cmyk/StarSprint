#!/usr/bin/env sh
set -eu

main_c="src/main.c"

# Guard against uploading raw packaged bytes directly.
rg -n "oamInitGfxSet\(\(u8 \*\)sprite_tiles" "${main_c}" >/dev/null && {
  echo "Unexpected direct sprite_tiles upload in init_video"
  exit 1
}

# Ensure conversion + converted upload both remain in place.
rg -n "convert_interleaved_4bpp_to_snes\(sprite_tiles, spriteTilesVram, SPRITE_TILES_LEN\);" "${main_c}" >/dev/null
rg -n "oamInitGfxSet\(\(u8 \*\)spriteTilesVram, SPRITE_TILES_LEN" "${main_c}" >/dev/null

echo "test_sprite_upload_path_regression: ok"
