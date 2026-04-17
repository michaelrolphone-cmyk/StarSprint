import re
import unittest
from pathlib import Path


class MainLoopTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_main_loop_reads_controllers_directly_without_scanpads(self):
        self.assertIn("pad0 = padsCurrent(0);", self.source)
        self.assertIn("pad1 = padsCurrent(1);", self.source)
        self.assertNotIn("scanPads();", self.source)

    def test_nmi_callback_flushes_console_via_dma_path(self):
        callback_match = re.search(
            r"static void vblank_dma_transfer\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(callback_match, "vblank_dma_transfer() not found")
        callback_body = callback_match.group("body")
        self.assertIn("consoleUpdate();", callback_body)
        self.assertIn("nmiSet(vblank_dma_transfer);", self.source)

    def test_main_loop_waits_for_vblank_before_input(self):
        match = re.search(r"while \(1\) \{(?P<body>.*?)\n    \}", self.source, re.S)
        self.assertIsNotNone(match, "main loop while(1) body not found")

        body = match.group("body")
        wait_index = body.find("WaitForVBlank();")
        pad1_index = body.find("pad1 = padsCurrent(1);")
        pads_index = body.find("pad0 = padsCurrent(0);")

        self.assertNotEqual(wait_index, -1, "WaitForVBlank() missing from main loop")
        self.assertNotEqual(pad1_index, -1, "padsCurrent(1) missing from main loop")
        self.assertNotEqual(pads_index, -1, "padsCurrent(0) missing from main loop")
        self.assertLess(wait_index, pads_index)
        self.assertLess(wait_index, pad1_index)

    def test_main_loop_waits_once_per_frame(self):
        match = re.search(r"while \(1\) \{(?P<body>.*?)\n    \}", self.source, re.S)
        self.assertIsNotNone(match, "main loop while(1) body not found")
        body = match.group("body")
        self.assertEqual(body.count("WaitForVBlank();"), 1)

    def test_play_hud_uses_dirty_cache_to_avoid_redraw_every_frame(self):
        draw_hud = re.search(
            r"static void draw_play_hud\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(draw_hud, "draw_play_hud() not found")
        body = draw_hud.group("body")
        self.assertIn("if (!playHudDirty &&", body)
        self.assertIn("return;", body)
        self.assertIn("playHudDirty = 0;", body)

    def test_non_play_screens_use_text_dirty_flags(self):
        for fn_name, dirty_flag in [
            ("draw_title_screen", "titleTextDirty"),
            ("draw_level_clear_screen", "levelClearTextDirty"),
            ("draw_all_clear_screen", "allClearTextDirty"),
        ]:
            fn = re.search(
                rf"static void {fn_name}\(void\) \{{(?P<body>.*?)\n\}}",
                self.source,
                re.S,
            )
            self.assertIsNotNone(fn, f"{fn_name}() not found")
            body = fn.group("body")
            self.assertIn(f"if (!{dirty_flag}) return;", body)
            self.assertIn(f"{dirty_flag} = 0;", body)

    def test_world_map_uses_cache_to_avoid_full_text_redraws(self):
        fn = re.search(
            r"static void draw_world_map\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(fn, "draw_world_map() not found")
        body = fn.group("body")
        self.assertIn("if (!worldMapTextDirty &&", body)
        self.assertIn("worldMapCachedSelected == selectedLevel", body)
        self.assertIn("worldMapCachedCompletedBits == completedBits", body)
        self.assertIn("worldMapCachedReserveSeconds == reserveSeconds", body)
        self.assertIn("worldMapTextDirty = 0;", body)

    def test_world_background_uses_tile_cache_and_fine_scroll(self):
        fn = re.search(
            r"static void draw_world_background\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(fn, "draw_world_background() not found")
        body = fn.group("body")
        self.assertIn("s16 fineScrollX = cameraX & (TILE_SIZE - 1);", body)
        self.assertIn("if (tx0 != worldBgCachedTx0) {", body)
        self.assertIn("for (tx = 0; tx <= (SCREEN_W / TILE_SIZE); tx++) {", body)
        self.assertIn("bgInitMapSet(1, (u8 *)worldBgMap, sizeof(worldBgMap), SC_32x32, BG_WORLD_MAP_VRAM_ADDR);", body)
        self.assertIn("worldBgCachedTx0 = tx0;", body)
        self.assertIn("bgSetScroll(1, fineScrollX, 0);", body)

    def test_world_background_tile_cache_resets_when_building_level(self):
        fn = re.search(
            r"static void build_level\(u8 levelIndex\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(fn, "build_level() not found")
        self.assertIn("worldBgCachedTx0 = -1;", fn.group("body"))

    def test_state_change_marks_hud_dirty_after_text_clear(self):
        state_change = re.search(
            r"if \(gameState != lastState\) \{(?P<body>.*?)\n        \}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(state_change, "state-change branch not found")
        body = state_change.group("body")
        self.assertIn("clear_text_screen();", body)
        self.assertIn("playHudDirty = 1;", body)
        self.assertIn("titleTextDirty = (gameState == STATE_TITLE);", body)
        self.assertIn("worldMapTextDirty = (gameState == STATE_WORLD_MAP);", body)
        self.assertIn("levelClearTextDirty = (gameState == STATE_LEVEL_CLEAR);", body)
        self.assertIn("allClearTextDirty = (gameState == STATE_ALL_CLEAR);", body)

    def test_entity_draw_loops_skip_offscreen_sprites_before_emit(self):
        for fn_name, guard in [
            ("draw_stars", "if (sx <= -16 || sx >= SCREEN_W) continue;"),
            ("draw_enemies", "if (sx <= -ENEMY_W || sx >= SCREEN_W) continue;"),
            ("draw_powerups", "if (sx <= -POWER_W || sx >= SCREEN_W) continue;"),
            ("draw_bolts", "if (sx <= -BOLT_W || sx >= SCREEN_W) continue;"),
        ]:
            fn = re.search(
                rf"static void {fn_name}\(void\) \{{(?P<body>.*?)\n\}}",
                self.source,
                re.S,
            )
            self.assertIsNotNone(fn, f"{fn_name}() not found")
            self.assertIn(guard, fn.group("body"))


if __name__ == "__main__":
    unittest.main()
