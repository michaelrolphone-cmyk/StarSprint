import re
import unittest
from pathlib import Path


class ParallaxBackgroundTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_play_state_draws_parallax_before_world_geometry(self):
        play_branch = re.search(
            r"else if \(gameState == STATE_PLAY\) \{(?P<body>.*?)\n        \} else if \(gameState == STATE_WORLD_MAP\)",
            self.source,
            re.S,
        )
        self.assertIsNotNone(play_branch, "play-state branch not found")
        body = play_branch.group("body")
        self.assertIn("draw_level_parallax_background();", body)
        self.assertLess(body.find("draw_level_parallax_background();"), body.find("draw_world();"))

    def test_parallax_function_has_cloud_mountain_tree_layers_with_distinct_speeds(self):
        fn = re.search(
            r"static void draw_level_parallax_background\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(fn, "draw_level_parallax_background() not found")
        body = fn.group("body")
        self.assertIn("cloudAnchors", body)
        self.assertIn("mountainAnchors", body)
        self.assertIn("treeAnchors", body)
        self.assertIn("((s16)cameraX >> 2)", body)
        self.assertIn("((s16)cameraX >> 1)", body)
        self.assertIn("(((s16)cameraX * 3) >> 2)", body)
        self.assertIn("SPR_STAR_SMILE", body)
        self.assertIn("SPR_BRICK", body)
        self.assertIn("SPR_GROW_POWER", body)


if __name__ == "__main__":
    unittest.main()
