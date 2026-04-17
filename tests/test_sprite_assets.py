import re
import unittest
from pathlib import Path


class SpriteAssetsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/assets.c").read_text()

    def _parse_hex_array(self, name: str):
        match = re.search(rf"const unsigned (?:char|short) {name}\[\] = \{{(?P<body>.*?)\}};", self.source, re.S)
        self.assertIsNotNone(match, f"{name} array not found")
        return [int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]+", match.group("body"))]

    def test_sprite_tile_payload_matches_declared_length(self):
        tiles = self._parse_hex_array("sprite_tiles")
        self.assertEqual(len(tiles), 2048, "sprite_tiles must remain 2048 bytes for 16 frames")

    def test_sprite_palette_uses_all_16_entries(self):
        palette = self._parse_hex_array("sprite_pal")
        self.assertEqual(len(palette), 16, "sprite_pal must contain 16 SNES colors")
        self.assertGreaterEqual(len(set(palette)), 12, "palette should contain distinct colors for the imported sprite style")

    def test_expected_signature_colors_exist(self):
        palette = set(self._parse_hex_array("sprite_pal"))
        # white, red/orange family, and bright yellow are used heavily in the provided spritesheet style.
        self.assertIn(0x7FFF, palette, "palette should include white highlights")
        self.assertIn(0x14FD, palette, "palette should include astronaut red accents")
        self.assertIn(0x239F, palette, "palette should include star yellow")


if __name__ == "__main__":
    unittest.main()
