# Sprite Rendering Debug Log

## Goal
Fix broken in-game sprite rendering without repeating failed experiments.

## Failed solutions (do not repeat)

1. **Runtime byte-lane conversion for every sprite tile upload**
   - Attempt: Convert `sprite_tiles` via `convert_interleaved_4bpp_to_snes(...)` before `oamInitGfxSet(...)`.
   - Result: Sprites still rendered incorrectly in-game.
   - Why this is considered failed: The conversion step did not resolve the visual corruption reported by the user.

## Working direction

- Upload the packaged sprite tile buffer directly to OAM (`sprite_tiles`) instead of reformatting it at runtime.
- Keep regression tests focused on sprite frame offset math and data-size bounds.
