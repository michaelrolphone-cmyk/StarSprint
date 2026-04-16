#include "game_logic.h"

uint8_t should_attempt_rope_grab(uint8_t onRope, uint8_t onGround, uint8_t ropeGrabLock) {
    return (onRope == 0 && onGround == 0 && ropeGrabLock == 0) ? 1 : 0;
}

uint8_t should_run_secondary_substep(int16_t playerVx, int16_t playerVy, uint8_t superActive, uint8_t activeBolts) {
    int16_t absVx = (playerVx < 0) ? -playerVx : playerVx;
    int16_t absVy = (playerVy < 0) ? -playerVy : playerVy;

    if (superActive) return 1;
    if (activeBolts) return 1;
    if (absVx >= 6) return 1;
    if (absVy >= 8) return 1;
    return 0;
}
