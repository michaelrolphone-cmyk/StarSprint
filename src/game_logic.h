#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

uint8_t should_attempt_rope_grab(uint8_t onRope, uint8_t onGround, uint8_t ropeGrabLock);
uint8_t should_run_secondary_substep(int16_t playerVx, int16_t playerVy, uint8_t superActive, uint8_t activeBolts);

#endif
