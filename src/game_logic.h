#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

#define PLAYER_MODE_COOP 0
#define PLAYER_MODE_TURN_BASED 1

#define SFX_NONE 0
#define SFX_JUMP 1
#define SFX_STAR 2
#define SFX_POWERUP 3
#define SFX_FIRE 4
#define SFX_HIT 5
#define SFX_ENEMY_STOMP 6
#define SFX_LEVEL_CLEAR 7
#define SFX_TURN_SWAP 8

#define STAR_METER_MAX 100
#define PLAYER_START_LIVES 5

#define UI_THEME_TITLE 0
#define UI_THEME_MAP 1
#define UI_THEME_PLAY 2
#define UI_THEME_CLEAR 3

uint8_t should_attempt_rope_grab(uint8_t onRope, uint8_t onGround, uint8_t ropeGrabLock);
uint8_t should_run_secondary_substep(int16_t playerVx, int16_t playerVy, uint8_t superActive, uint8_t activeBolts);
uint16_t merge_coop_input(uint16_t pad0, uint16_t pad1, uint8_t mode, uint8_t activeTurnPlayer);
uint8_t next_turn_player(uint8_t mode, uint8_t activeTurnPlayer, uint8_t swapRequested);
uint8_t sound_for_event(uint8_t eventId);
uint8_t ui_theme_for_state(uint8_t gameState);
uint8_t level_style_seed(uint8_t levelIndex, uint8_t salt);
void award_star_and_super(uint8_t *starsTowardCharge, uint16_t *superReserveFrames, uint16_t framesPerCharge);
uint8_t grant_extra_life(uint8_t lives, uint8_t maxLives);
uint8_t lose_life_and_continue(uint8_t *lives);

#endif
