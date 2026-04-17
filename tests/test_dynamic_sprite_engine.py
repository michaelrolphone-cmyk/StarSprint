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

    def test_world_tiles_render_with_bg_tilemap_instead_of_oam_budget(self):
        self.assertIn("#define BG_WORLD_MAP_W 64", self.main_source)
        self.assertIn("static u16 worldBgMap[BG_WORLD_MAP_W * BG_WORLD_MAP_H];", self.main_source)
        self.assertIn("static void draw_world_background(void)", self.main_source)
        self.assertIn("#define SPRITE_FRAME_COUNT (SPRITE_TILES_LEN / (SPRITE_16X16_TILE_COUNT * SPRITE_BYTES_PER_8X8))", self.main_source)
        self.assertIn("#define WORLD_BG_TILE_ATTR(tileIndex) ((u16)(tileIndex))", self.main_source)
        self.assertIn("#define WORLD_BG_EMPTY_TILE_BASE ((u16)((SPRITE_FRAME_COUNT - 1) * SPRITE_16X16_TILE_COUNT))", self.main_source)
        self.assertIn("bgInitTileSet(1, (u8 *)sprite_tiles, (u8 *)sprite_pal, 0, SPRITE_TILES_LEN, SPRITE_PAL_LEN, BG_16COLORS, BG_WORLD_TILE_VRAM_ADDR);", self.main_source)
        self.assertIn("bgInitMapSet(1, (u8 *)worldBgMap, sizeof(worldBgMap), SC_64x32, BG_WORLD_MAP_VRAM_ADDR);", self.main_source)
        self.assertIn("draw_world_background();", self.main_source)
        self.assertNotIn("draw_world();", self.main_source)

    def test_bg_map_size_matches_17_visible_tiles_as_16x16_quads(self):
        self.assertIn("for (tx = 0; tx <= (SCREEN_W / TILE_SIZE); tx++) {", self.main_source)
        self.assertIn("#define BG_WORLD_MAP_W 64", self.main_source)
        self.assertIn("bgSetMapPtr(1, BG_WORLD_MAP_VRAM_ADDR, SC_64x32);", self.main_source)

    def test_world_tilemap_uses_tile_attributes_for_all_quadrants(self):
        fn = re.search(r"static void world_bg_put_16x16\(u8 mx, u8 my, u16 tileBase\) \{(?P<body>.*?)\n\}", self.main_source, re.S)
        self.assertIsNotNone(fn, "world_bg_put_16x16() not found")
        body = fn.group("body")
        self.assertIn("worldBgMap[row + mx] = WORLD_BG_TILE_ATTR(tileBase);", body)
        self.assertIn("worldBgMap[row + mx + 1] = WORLD_BG_TILE_ATTR(tileBase + 1);", body)
        self.assertIn("worldBgMap[row + BG_WORLD_MAP_W + mx] = WORLD_BG_TILE_ATTR(tileBase + 2);", body)
        self.assertIn("worldBgMap[row + BG_WORLD_MAP_W + mx + 1] = WORLD_BG_TILE_ATTR(tileBase + 3);", body)

    def test_world_bg_map_clear_uses_blank_tile_not_tile_zero(self):
        fn = re.search(r"static void clear_world_bg_map\(void\) \{(?P<body>.*?)\n\}", self.main_source, re.S)
        self.assertIsNotNone(fn, "clear_world_bg_map() not found")
        body = fn.group("body")
        self.assertIn("worldBgMap[i] = WORLD_BG_TILE_ATTR(WORLD_BG_EMPTY_TILE_BASE);", body)

    def test_frame_lifecycle_uses_nmi_console_flush_without_manual_oam_dma(self):
        self.assertIn("consoleVblank();", self.main_source)
        self.assertNotIn("oamUpdate();", self.main_source)
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
        self.assertIn("#define SPRITE_GFX_OFFSET(frame) ((u16)(frame) * SPRITE_16X16_TILE_COUNT)", self.main_source)

    def test_imported_palette_keeps_astronaut_and_space_tones(self):
        match = re.search(r"const unsigned short sprite_pal\[\] = \{(?P<body>.*?)\};", self.assets_source, re.S)
        self.assertIsNotNone(match, "sprite_pal array not found")
        palette = {int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]+", match.group("body"))}

        self.assertIn(0x14FD, palette, "palette should include astronaut red accents")
        self.assertIn(0x51AA, palette, "palette should include purple tones for hero/villain sprites")
        self.assertIn(0x02A0, palette, "palette should include green tones from the sheet")


if __name__ == "__main__":
    unittest.main()
