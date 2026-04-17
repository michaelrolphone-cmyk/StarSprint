import re
import unittest
from pathlib import Path


class RopeReleaseAndPacingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_player_state_tracks_rope_regrab_cooldown(self):
        player_struct = re.search(r"typedef struct \{(?P<body>.*?)\n\} Player;", self.source, re.S)
        self.assertIsNotNone(player_struct, "Player struct not found")
        self.assertIn("u8 ropeRegrabCooldown;", player_struct.group("body"))

    def test_rope_jump_sets_cooldown_before_leaving_rope(self):
        update_input = re.search(
            r"static void update_player_input\(u8 playerIndex, u16 padCur, u16 padOld\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(update_input, "update_player_input() not found")
        body = update_input.group("body")
        self.assertIn("p->ropeRegrabCooldown = ROPE_REGRAB_COOLDOWN;", body)

    def test_rope_grab_respects_regrab_cooldown(self):
        try_grab = re.search(r"static void try_grab_rope\(Player \*p\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(try_grab, "try_grab_rope() not found")
        self.assertIn("if (p->onRope || p->onGround || p->ropeRegrabCooldown) return;", try_grab.group("body"))

    def test_move_player_ticks_regrab_cooldown_every_frame(self):
        move_fn = re.search(r"static void move_player\(u8 playerIndex\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(move_fn, "move_player() not found")
        self.assertIn("if (p->ropeRegrabCooldown) p->ropeRegrabCooldown--;", move_fn.group("body"))

    def test_speed_constants_are_tuned_for_faster_pacing(self):
        self.assertIn("#define SPEED_WALK 5", self.source)
        self.assertIn("#define SPEED_RUN 8", self.source)
        self.assertIn("#define SPEED_SUPER 14", self.source)


if __name__ == "__main__":
    unittest.main()
