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
        self.assertIn('"B JUMP A RUN/FIRE Y WALL DOWN SLD"', self.source)

    def test_camera_drag_pulls_players_to_left_screen_edge(self):
        drag_fn = re.search(r"static void apply_coop_screen_drag\(void\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(drag_fn, "apply_coop_screen_drag() not found")
        body = drag_fn.group("body")
        self.assertIn("if (players[i].x < leftEdge)", body)
        self.assertIn("s16 dragDelta = leftEdge - players[i].x;", body)
        self.assertIn("players[i].x = leftEdge;", body)
        self.assertIn("if (players[i].vx < 0) players[i].vx = 0;", body)
        self.assertIn("if (players[i].holding < MAX_PLAYERS)", body)
        self.assertIn("held->x += dragDelta;", body)

    def test_star_pickup_uses_padding_to_prevent_fast_movement_misses(self):
        self.assertIn("#define STAR_COLLECT_PADDING_X 3", self.source)
        self.assertIn("#define STAR_COLLECT_PADDING_Y 3", self.source)
        self.assertIn("p->x - STAR_COLLECT_PADDING_X", self.source)
        self.assertIn("p->y - STAR_COLLECT_PADDING_Y", self.source)

    def test_play_loop_applies_screen_drag_after_camera_update(self):
        play_state = re.search(
            r"else if \(gameState == STATE_PLAY\) \{(?P<body>.*?)\n        \}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(play_state, "STATE_PLAY branch not found")
        body = play_state.group("body")
        self.assertIn("update_camera();", body)
        self.assertIn("apply_coop_screen_drag();", body)
        self.assertLess(body.find("update_camera();"), body.find("apply_coop_screen_drag();"))

    def test_camera_tracks_leading_player_for_forward_drag(self):
        camera_fn = re.search(r"static void update_camera\(void\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(camera_fn, "update_camera() not found")
        body = camera_fn.group("body")
        self.assertIn("#define COOP_CAMERA_LEAD_OFFSET 112", self.source)
        self.assertIn("s16 focusX = (player.x > player2.x) ? player.x : player2.x;", body)
        self.assertIn("s16 target = focusX - COOP_CAMERA_LEAD_OFFSET;", body)

    def test_players_can_land_on_each_others_heads(self):
        head_fn = re.search(r"static void resolve_player_head_stand\(Player \*rider, Player \*base\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(head_fn, "resolve_player_head_stand() not found")
        body = head_fn.group("body")
        self.assertIn("if (rider->vy < 0) return;", body)
        self.assertIn("if ((overlapRight - overlapLeft) < 6) return;", body)
        self.assertIn("rider->y = baseTop - riderH;", body)
        self.assertIn("rider->vy = 0;", body)
        self.assertIn("rider->onGround = 1;", body)
        self.assertIn("if (rider->smash)", body)
        self.assertIn("bounce_player_from_smash(base, rider);", body)

    def test_smash_has_faster_fall_and_bounce_distance(self):
        self.assertIn("#define SMASH_FALL_MAX 18", self.source)
        self.assertIn("#define SMASH_BOUNCE_DISTANCE 24", self.source)
        self.assertIn("#define SMASH_BOUNCE_UPWARD -9", self.source)
        self.assertIn("s16 maxFall = p->smash ? SMASH_FALL_MAX : MAX_FALL;", self.source)
        self.assertIn("if (p->smash && p->vy > 0 && (p->y & 1) == 0) p->vy++;", self.source)

    def test_smash_landing_pushes_target_player_away(self):
        smash_fn = re.search(r"static void bounce_player_from_smash\(Player \*launched, const Player \*source\) \{(?P<body>.*?)\n\}", self.source, re.S)
        self.assertIsNotNone(smash_fn, "bounce_player_from_smash() not found")
        body = smash_fn.group("body")
        self.assertIn("targetX = launched->x + (direction * SMASH_BOUNCE_DISTANCE);", body)
        self.assertIn("launched->vy = SMASH_BOUNCE_UPWARD;", body)
        self.assertIn("launched->onGround = 0;", body)

    def test_play_loop_resolves_player_stacking_after_player_movement(self):
        play_state = re.search(
            r"else if \(gameState == STATE_PLAY\) \{(?P<body>.*?)\n        \}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(play_state, "STATE_PLAY branch not found")
        body = play_state.group("body")
        self.assertIn("move_player(0);", body)
        self.assertIn("move_player(1);", body)
        self.assertIn("resolve_player_stack_collision();", body)
        self.assertLess(body.find("move_player(1);"), body.find("resolve_player_stack_collision();"))
        self.assertLess(body.find("resolve_player_stack_collision();"), body.find("update_enemies();"))


if __name__ == "__main__":
    unittest.main()
