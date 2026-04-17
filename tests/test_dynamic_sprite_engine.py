import re
import unittest
from pathlib import Path


class DynamicSpriteEngineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main_source = Path("src/main.c").read_text()
        cls.assets_source = Path("src/assets.c").read_text()

    def test_video_init_uses_dynamic_sprite_engine(self):
        self.assertIn("oamInitDynamicSprite(DYNAMIC_SPRITE_GFX0, DYNAMIC_SPRITE_GFX1, 0, 0, OBJ_SIZE16_L32);", self.main_source)
        self.assertIn("setPalette((u8 *)sprite_pal, 128 + (0 * 16), SPRITE_PAL_LEN);", self.main_source)

    def test_sprite_emit_writes_dynamic_oam_buffer_and_draws(self):
        fn = re.search(r"static void sprite_emit\(u8 frame, s16 sx, s16 sy, u8 hflip, u8 pal\) \{(?P<body>.*?)\n\}", self.main_source, re.S)
        self.assertIsNotNone(fn, "sprite_emit() not found")
        body = fn.group("body")
        self.assertIn("oambuffer[spriteCount].oamx = sx;", body)
        self.assertIn("oambuffer[spriteCount].oamy = sy;", body)
        self.assertIn("oambuffer[spriteCount].oamrefresh = 1;", body)
        self.assertIn("oambuffer[spriteCount].oamgraphics = ((u8 *)sprite_tiles) + ((u16)frame * SPRITE_FRAME_BYTES);", body)
        self.assertIn("oamDynamic16Draw(spriteCount);", body)

    def test_dynamic_sprite_frame_lifecycle_runs_every_frame(self):
        self.assertIn("oamVramQueueUpdate();", self.main_source)
        self.assertIn("oamUpdate();", self.main_source)
        self.assertIn("oamInitDynamicSpriteEndFrame();", self.main_source)

    def test_imported_palette_keeps_astronaut_and_space_tones(self):
        match = re.search(r"const unsigned short sprite_pal\[\] = \{(?P<body>.*?)\};", self.assets_source, re.S)
        self.assertIsNotNone(match, "sprite_pal array not found")
        palette = {int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]+", match.group("body"))}

        self.assertIn(0x14FD, palette, "palette should include astronaut red accents")
        self.assertIn(0x51AA, palette, "palette should include purple tones for hero/villain sprites")
        self.assertIn(0x02A0, palette, "palette should include green tones from the sheet")


if __name__ == "__main__":
    unittest.main()
