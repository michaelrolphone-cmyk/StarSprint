import re
import unittest
from pathlib import Path


class MainLoopTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_main_loop_scans_pads_every_frame(self):
        self.assertIn("scanPads();", self.source)

    def test_nmi_callback_flushes_console_via_dma_path(self):
        callback_match = re.search(
            r"static void vblank_dma_transfer\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(callback_match, "vblank_dma_transfer() not found")
        callback_body = callback_match.group("body")
        self.assertIn("consoleVblank();", callback_body)
        self.assertIn("nmiSet(vblank_dma_transfer);", self.source)

    def test_main_loop_waits_for_vblank_before_input(self):
        match = re.search(r"while \(1\) \{(?P<body>.*?)\n    \}", self.source, re.S)
        self.assertIsNotNone(match, "main loop while(1) body not found")

        body = match.group("body")
        wait_index = body.find("WaitForVBlank();")
        scan_index = body.find("scanPads();")
        pads_index = body.find("pad0 = padsCurrent(0);")

        self.assertNotEqual(wait_index, -1, "WaitForVBlank() missing from main loop")
        self.assertNotEqual(scan_index, -1, "scanPads() missing from main loop")
        self.assertNotEqual(pads_index, -1, "padsCurrent(0) missing from main loop")
        self.assertLess(wait_index, scan_index)
        self.assertLess(scan_index, pads_index)

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
