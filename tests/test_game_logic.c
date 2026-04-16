#include <assert.h>
#include <stdio.h>

#include "../src/game_logic.h"

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

int main(void) {
    test_should_attempt_rope_grab();
    test_should_run_secondary_substep();
    puts("test_game_logic: ok");
    return 0;
}
