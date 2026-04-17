import re
import unittest
from pathlib import Path


class CoopModeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_second_controller_is_polled(self):
        self.assertIn("pad1 = padsCurrent(1);", self.source)
        self.assertIn("update_player_input(1, pad1, padPrev1);", self.source)

    def test_y_button_pickup_and_throw_logic_exists(self):
        self.assertIn("u8 yPressed = ((padCur & KEY_Y) && !(padOld & KEY_Y));", self.source)
        self.assertIn("if (p->holding < MAX_PLAYERS)", self.source)
        self.assertIn("release_hold(playerIndex, 1);", self.source)

    def test_held_player_can_jump_to_escape(self):
        update_input = re.search(
            r"static void update_player_input\(u8 playerIndex, u16 padCur, u16 padOld\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(update_input, "update_player_input() not found")
        held_block = re.search(r"if \(p->heldBy < MAX_PLAYERS\) \{(?P<body>.*?)\n    \}", update_input.group("body"), re.S)
        self.assertIsNotNone(held_block, "held-player branch not found")
        body = held_block.group("body")
        self.assertIn("if (jumpPressed)", body)
        self.assertIn("release_hold(holder, 0);", body)
        self.assertIn("p->vy = JUMP_VELOCITY;", body)

    def test_hud_mentions_pickup_throw_control(self):
        self.assertIn('"B JUMP  A RUN/FIRE  Y PICKUP/WALL"', self.source)


if __name__ == "__main__":
    unittest.main()
