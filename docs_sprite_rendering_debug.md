# Sprite Rendering Debug Log

## Goal
Fix broken in-game sprite rendering without repeating failed experiments.

## Failed solutions (do not repeat)

1. **Runtime byte-lane conversion for every sprite tile upload**
   - Attempt: Convert `sprite_tiles` via `convert_interleaved_4bpp_to_snes(...)` before `oamInitGfxSet(...)`.
   - Result: Sprites still rendered incorrectly in-game.
   - Why this is considered failed: The conversion step did not resolve the visual corruption reported by the user.

2. **Packed atlas frame-offset rollback only**
   - Attempt: Changed `sprite_frame_offset_16x16(...)` from linear `frameIndex * 4` back to packed addressing `((frameIndex >> 3) * 32) + ((frameIndex & 7) * 2)`.
   - Result: User still reports sprites are broken.
   - Why this is considered failed: Tile-index math alone is not the full root cause, so additional rendering pipeline checks are required.

3. **Header-macro API aliasing for text VRAM functions**
   - Attempt: Added compatibility `#define` aliases so `consoleSetTextGfxPtr` always rewrote to `consoleSetTextVramBGAdr` and `consoleSetTextMapPtr` to `consoleSetTextVramAdr`.
   - Result: Sprite rendering remained broken.
   - Why this is considered failed: `#ifndef` checks macro existence (not function existence), so the rewrite always happened and routed init through the wrong API mapping for the active toolchain, corrupting VRAM layout.

## Root cause fixed

- Removed forced preprocessor aliasing for console text VRAM APIs in `src/main.c`.
- Kept direct calls in `init_video()` (`consoleSetTextGfxPtr` + `consoleSetTextMapPtr`) matching the last-known working code path.
- Added a regression check to ensure the broken alias block does not return.
- Restored explicit sprite-tile lane conversion before OAM upload and upload from the converted buffer.
- Restored linear frame addressing (`frameIndex * 4`) so each 16x16 frame maps to its contiguous 2x2 tile block.


## Current hypotheses to validate

- Runtime conversion may be necessary, but frame indexing must match packed atlas rows.
- The next fix attempts packed frame offsets while keeping the known-good text VRAM init path intact.
