#ifndef STARSPRINT_ASSETS_H
#define STARSPRINT_ASSETS_H

#include <snes.h>

enum SpriteFrame {
    // Keep these numeric frame IDs in sync with sprite_tiles[] packing order in assets.c.
    SPR_PLAYER_SMALL = 0,
    SPR_PLAYER_BIG_TOP = 1,
    SPR_PLAYER_BIG_BOTTOM = 2,
    SPR_ENEMY_WALKER = 3,
    SPR_ENEMY_HOPPER = 4,
    SPR_USED_BLOCK = 5,
    SPR_BRICK = 6,
    SPR_STAR_SMILE = 7,
    SPR_GROW_POWER = 8,
    SPR_LIGHT_POWER = 9,
    SPR_BOLT = 10,
    SPR_GROUND = 11,
    SPR_SPIKES = 12,
};

extern const unsigned char sprite_tiles[];
extern const unsigned short sprite_pal[];

#define SPRITE_TILES_LEN 2048
#define SPRITE_PAL_LEN 32

#endif
