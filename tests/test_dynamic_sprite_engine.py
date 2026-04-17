import re
import unittest
from pathlib import Path


class DynamicSpriteEngineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main_source = Path("src/main.c").read_text()
        cls.assets_source = Path("src/assets.c").read_text()

    def test_video_init_uses_static_sprite_sheet_pipeline(self):
        self.assertIn(
            "oamInitGfxSet((u8 *)sprite_tiles, SPRITE_TILES_LEN, (u8 *)sprite_pal, SPRITE_PAL_LEN, 0, SPRITE_GFX_VRAM_ADDR, OBJ_SIZE16_L32);",
            self.main_source,
        )

    def test_sprite_emit_uses_oamset_and_sprite_size_flags(self):
        fn = re.search(r"static void sprite_emit\(u8 frame, s16 sx, s16 sy, u8 hflip, u8 pal\) \{(?P<body>.*?)\n\}", self.main_source, re.S)
        self.assertIsNotNone(fn, "sprite_emit() not found")
        body = fn.group("body")
        self.assertIn("oamId = spriteCount * 4;", body)
        self.assertIn("oamSet(oamId, sx, sy, 3, hflip ? 1 : 0, 0, SPRITE_GFX_OFFSET(frame), pal & 0x07);", body)
        self.assertIn("oamSetEx(oamId, OBJ_SMALL, OBJ_SHOW);", body)

    def test_frame_lifecycle_updates_oam_without_dynamic_upload_queue(self):
        self.assertIn("oamUpdate();", self.main_source)
        self.assertNotIn("oamVramQueueUpdate();", self.main_source)
        self.assertNotIn("oamInitDynamicSpriteEndFrame();", self.main_source)

    def test_sprite_end_hides_remaining_oam_entries_by_oam_id(self):
        fn = re.search(r"static void sprite_end\(void\) \{(?P<body>.*?)\n\}", self.main_source, re.S)
        self.assertIsNotNone(fn, "sprite_end() not found")
        body = fn.group("body")
        self.assertIn("oamSetEx(spriteCount * 4, OBJ_SMALL, OBJ_HIDE);", body)

    def test_sprite_offset_macro_matches_16x16_tile_layout(self):
        self.assertIn("#define SPRITE_BYTES_PER_8X8 32", self.main_source)
        self.assertIn("#define SPRITE_16X16_TILE_COUNT 4", self.main_source)
        self.assertIn("#define SPRITE_GFX_OFFSET(frame) ((u16)(frame) * SPRITE_16X16_TILE_COUNT * SPRITE_BYTES_PER_8X8)", self.main_source)

    def test_imported_palette_keeps_astronaut_and_space_tones(self):
        match = re.search(r"const unsigned short sprite_pal\[\] = \{(?P<body>.*?)\};", self.assets_source, re.S)
        self.assertIsNotNone(match, "sprite_pal array not found")
        palette = {int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]+", match.group("body"))}

        self.assertIn(0x14FD, palette, "palette should include astronaut red accents")
        self.assertIn(0x51AA, palette, "palette should include purple tones for hero/villain sprites")
        self.assertIn(0x02A0, palette, "palette should include green tones from the sheet")


if __name__ == "__main__":
    unittest.main()
