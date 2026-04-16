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

## Working direction

- Upload the packaged sprite tile buffer directly to OAM (`sprite_tiles`) instead of reformatting it at runtime.
- Keep regression tests focused on sprite frame offset math and data-size bounds.
- Next debugging step: capture and compare a VRAM/OAM dump from startup to confirm whether corruption is introduced during upload or during per-frame OAM writes.
