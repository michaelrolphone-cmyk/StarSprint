import re
import unittest
from pathlib import Path


class SpriteFrameMappingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assets_header = Path("src/assets.h").read_text()
        cls.main_source = Path("src/main.c").read_text()

    def test_sprite_frame_constants_match_assets_sheet_order(self):
        expected = {
            "SPR_PLAYER_SMALL": 0,
            "SPR_PLAYER_BIG_TOP": 1,
            "SPR_PLAYER_BIG_BOTTOM": 2,
            "SPR_ENEMY_WALKER": 3,
            "SPR_ENEMY_HOPPER": 4,
            "SPR_USED_BLOCK": 5,
            "SPR_BRICK": 6,
            "SPR_STAR_SMILE": 7,
            "SPR_GROW_POWER": 8,
            "SPR_LIGHT_POWER": 9,
            "SPR_BOLT": 10,
            "SPR_GROUND": 11,
            "SPR_SPIKES": 12,
        }

        for name, value in expected.items():
            pattern = rf"{name}\s*=\s*{value}\s*,"
            self.assertRegex(
                self.assets_header,
                pattern,
                f"{name} must stay mapped to frame {value}",
            )

    def test_world_background_mapping_uses_block_frame_constants(self):
        fn = re.search(
            r"static u16 world_bg_tile_base\(u8 tile\) \{(?P<body>.*?)\n\}",
            self.main_source,
            re.S,
        )
        self.assertIsNotNone(fn, "world_bg_tile_base() not found")
        body = fn.group("body")
        self.assertIn("case TILE_BRICK:", body)
        self.assertIn("frame = SPR_BRICK;", body)
        self.assertIn("case TILE_USED:", body)
        self.assertIn("frame = SPR_USED_BLOCK;", body)


if __name__ == "__main__":
    unittest.main()
