import unittest
from pathlib import Path


class BuildRegressionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.makefile = Path("Makefile").read_text()
        cls.main_source = Path("src/main.c").read_text()
        cls.gitignore = Path(".gitignore").read_text()

    def test_makefile_filters_generated_asset_sources_with_and_without_src_prefix(self):
        self.assertIn("assets.asm", self.makefile)
        self.assertIn("$(SRC)/assets.asm", self.makefile)
        self.assertIn("assets.asp", self.makefile)
        self.assertIn("$(SRC)/assets.asp", self.makefile)
        self.assertIn("assets.ps", self.makefile)
        self.assertIn("$(SRC)/assets.ps", self.makefile)
        self.assertIn("assets.obj $(SRC)/assets.obj", self.makefile)

    def test_world_bg_macro_uses_raw_tile_indices_for_bg_map_words(self):
        self.assertIn("#define WORLD_BG_TILE_ATTR(tileIndex) ((u16)(tileIndex))", self.main_source)
        self.assertNotIn("TILE_ATTR_FULL(0, 0, 0, 0, (tileIndex))", self.main_source)

    def test_console_vblank_has_explicit_declaration(self):
        self.assertIn("extern void consoleVblank(void);", self.main_source)

    def test_gitignore_excludes_generated_asset_assembler_outputs(self):
        self.assertIn("src/assets.asm", self.gitignore)
        self.assertIn("src/assets.asp", self.gitignore)
        self.assertIn("src/assets.ps", self.gitignore)


if __name__ == "__main__":
    unittest.main()
