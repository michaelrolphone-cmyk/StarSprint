#include <assert.h>
#include <stdio.h>

#include "../src/game_logic.h"

#define KEY_LEFT 0x0040
#define KEY_RIGHT 0x0080
#define KEY_START 0x1000

static void test_should_attempt_rope_grab(void) {
    assert(should_attempt_rope_grab(0, 0, 0) == 1);
    assert(should_attempt_rope_grab(1, 0, 0) == 0);
    assert(should_attempt_rope_grab(0, 1, 0) == 0);
    assert(should_attempt_rope_grab(0, 0, 3) == 0);
}

static void test_should_run_secondary_substep(void) {
    assert(should_run_secondary_substep(0, 0, 0, 0) == 0);
    assert(should_run_secondary_substep(6, 0, 0, 0) == 1);
    assert(should_run_secondary_substep(0, -8, 0, 0) == 1);
    assert(should_run_secondary_substep(0, 0, 1, 0) == 1);
    assert(should_run_secondary_substep(0, 0, 0, 1) == 1);
}

static void test_merge_coop_input(void) {
    assert(merge_coop_input(KEY_LEFT, KEY_RIGHT, PLAYER_MODE_COOP, 0) == (KEY_LEFT | KEY_RIGHT));
    assert(merge_coop_input(KEY_LEFT, KEY_RIGHT, PLAYER_MODE_TURN_BASED, 0) == KEY_LEFT);
    assert(merge_coop_input(KEY_LEFT, KEY_RIGHT, PLAYER_MODE_TURN_BASED, 1) == KEY_RIGHT);
}

static void test_next_turn_player(void) {
    assert(next_turn_player(PLAYER_MODE_COOP, 1, 1) == 0);
    assert(next_turn_player(PLAYER_MODE_TURN_BASED, 0, 0) == 0);
    assert(next_turn_player(PLAYER_MODE_TURN_BASED, 0, 1) == 1);
    assert(next_turn_player(PLAYER_MODE_TURN_BASED, 1, 1) == 0);
}

static void test_sound_for_event(void) {
    assert(sound_for_event(SFX_JUMP) == SFX_JUMP);
    assert(sound_for_event(SFX_TURN_SWAP) == SFX_TURN_SWAP);
    assert(sound_for_event(99) == SFX_NONE);
}

static void test_award_star_and_super(void) {
    uint8_t stars = 99;
    uint16_t reserve = 0;
    award_star_and_super(&stars, &reserve, 3600);
    assert(stars == 0);
    assert(reserve == 3600);
}

static void test_grant_extra_life(void) {
    assert(grant_extra_life(5, 9) == 6);
    assert(grant_extra_life(9, 9) == 9);
}

static void test_lose_life_and_continue(void) {
    uint8_t lives = 2;
    assert(lose_life_and_continue(&lives) == 1);
    assert(lives == 1);
    assert(lose_life_and_continue(&lives) == 0);
    assert(lives == 0);
}

int main(void) {
    test_should_attempt_rope_grab();
    test_should_run_secondary_substep();
    test_merge_coop_input();
    test_next_turn_player();
    test_sound_for_event();
    test_award_star_and_super();
    test_grant_extra_life();
    test_lose_life_and_continue();
    puts("test_game_logic: ok");
    return 0;
}
