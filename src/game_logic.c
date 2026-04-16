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

uint16_t merge_coop_input(uint16_t pad0, uint16_t pad1, uint8_t mode, uint8_t activeTurnPlayer) {
    if (mode == PLAYER_MODE_COOP) return (uint16_t)(pad0 | pad1);
    return activeTurnPlayer ? pad1 : pad0;
}

uint8_t next_turn_player(uint8_t mode, uint8_t activeTurnPlayer, uint8_t swapRequested) {
    if (mode != PLAYER_MODE_TURN_BASED) return 0;
    if (!swapRequested) return activeTurnPlayer;
    return activeTurnPlayer ? 0 : 1;
}

uint8_t sound_for_event(uint8_t eventId) {
    if (eventId > SFX_TURN_SWAP) return SFX_NONE;
    return eventId;
}

void award_star_and_super(uint8_t *starsTowardCharge, uint16_t *superReserveFrames, uint16_t framesPerCharge) {
    if (!starsTowardCharge || !superReserveFrames) return;
    if (*starsTowardCharge < STAR_METER_MAX) {
        (*starsTowardCharge)++;
    }
    while (*starsTowardCharge >= STAR_METER_MAX) {
        *starsTowardCharge -= STAR_METER_MAX;
        *superReserveFrames = (uint16_t)(*superReserveFrames + framesPerCharge);
    }
}

uint8_t grant_extra_life(uint8_t lives, uint8_t maxLives) {
    if (lives < maxLives) return (uint8_t)(lives + 1);
    return lives;
}

uint8_t lose_life_and_continue(uint8_t *lives) {
    if (!lives || *lives == 0) return 0;
    (*lives)--;
    return (*lives > 0) ? 1 : 0;
}
