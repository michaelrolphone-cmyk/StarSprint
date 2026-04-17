import re
import unittest
from pathlib import Path


class TextRenderingConfigTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_console_init_uses_valid_palette_index(self):
        match = re.search(r"consoleInitText\((?P<args>[^;]+)\);", self.source)
        self.assertIsNotNone(match, "consoleInitText(...) call not found")
        args = [arg.strip() for arg in match.group("args").split(",")]
        self.assertGreaterEqual(len(args), 2, "consoleInitText should have at least two arguments")
        self.assertEqual(args[1], "0", "consoleInitText should initialize text on palette 0")

    def test_console_palette_upload_targets_palette_zero(self):
        self.assertIn("consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));", self.source)

    def test_text_background_layer_is_explicitly_enabled(self):
        self.assertIn("bgSetEnable(0);", self.source)


if __name__ == "__main__":
    unittest.main()
