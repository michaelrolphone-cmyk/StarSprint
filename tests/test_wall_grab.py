import re
import unittest
from pathlib import Path


class WallGrabTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_player_state_includes_wall_grab_fields(self):
        player_struct = re.search(r"typedef struct \{(?P<body>.*?)\n\} Player;", self.source, re.S)
        self.assertIsNotNone(player_struct, "Player struct not found")
        body = player_struct.group("body")
        self.assertIn("u8 wallHolding;", body)
        self.assertIn("s8 wallSide;", body)
        self.assertIn("u8 wantsWallGrab;", body)

    def test_input_sets_grab_intent_from_y_press(self):
        update_input = re.search(
            r"static void update_player_input\(u8 playerIndex, u16 padCur, u16 padOld\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(update_input, "update_player_input() not found")
        body = update_input.group("body")
        self.assertIn("u8 yPressed = ((padCur & KEY_Y) && !(padOld & KEY_Y));", body)
        self.assertIn("u8 yHeld = (padCur & KEY_Y) ? 1 : 0;", body)
        self.assertIn("p->wantsWallGrab = yPressed;", body)

    def test_wall_hold_jump_forces_away_from_wall(self):
        self.assertIn("if (p->wallHolding)", self.source)
        self.assertIn("p->vx = (p->wallSide > 0) ? -SPEED_RUN : SPEED_RUN;", self.source)

    def test_move_player_caps_wall_slide_speed_to_half_fall(self):
        move_fn = re.search(r"static void move_player\(u8 playerIndex\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(move_fn, "move_player() not found")
        body = move_fn.group("body")
        self.assertIn("if (p->wallHolding)", body)
        self.assertIn("if (p->vy < (MAX_FALL / 2)) p->vy += GRAVITY;", body)
        self.assertIn("if (p->vy > (MAX_FALL / 2)) p->vy = (MAX_FALL / 2);", body)

    def test_wall_grab_only_starts_when_pressing_y_into_wall(self):
        self.assertIn("if (p->wantsWallGrab && !p->onGround && !p->wallHolding)", self.source)
        self.assertIn("p->wallHolding = 1;", self.source)
        self.assertIn("p->wallSide = step;", self.source)


if __name__ == "__main__":
    unittest.main()
