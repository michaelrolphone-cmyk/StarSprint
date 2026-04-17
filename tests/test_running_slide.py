import re
import unittest
from pathlib import Path


class RunningSlideTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_player_state_tracks_slide_momentum(self):
        player_struct = re.search(r"typedef struct \{(?P<body>.*?)\n\} Player;", self.source, re.S)
        self.assertIsNotNone(player_struct, "Player struct not found")
        body = player_struct.group("body")
        self.assertIn("u8 sliding;", body)
        self.assertIn("s16 slideMomentum;", body)
        self.assertIn("s16 slideMaxMomentum;", body)

    def test_slide_starts_with_two_or_four_block_momentum(self):
        update_input = re.search(
            r"static void update_player_input\(u8 playerIndex, u16 padCur, u16 padOld\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(update_input, "update_player_input() not found")
        body = update_input.group("body")
        self.assertIn("u8 downPressed = ((padCur & KEY_DOWN) && !(padOld & KEY_DOWN));", body)
        self.assertIn("if (downPressed && p->onGround && (p->vx >= SPEED_RUN || p->vx <= -SPEED_RUN))", body)
        self.assertIn("p->slideMaxMomentum = superActive ? (TILE_SIZE * 4) : (TILE_SIZE * 2);", body)
        self.assertIn("p->slideMomentum = p->slideMaxMomentum;", body)

    def test_slide_loses_momentum_each_pixel_moved(self):
        move_fn = re.search(r"static void move_player\(u8 playerIndex\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(move_fn, "move_player() not found")
        body = move_fn.group("body")
        self.assertIn("if (p->sliding && p->slideMomentum > 0) p->slideMomentum--;", body)
        self.assertIn("if (p->sliding && p->slideMomentum <= 0)", body)
        self.assertIn("p->sliding = 0;", body)


if __name__ == "__main__":
    unittest.main()
