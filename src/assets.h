#ifndef STARSPRINT_ASSETS_H
#define STARSPRINT_ASSETS_H

#include <snes.h>

enum SpriteFrame {
    SPR_PLAYER_SMALL = 0,
    SPR_PLAYER_BIG_TOP,
    SPR_PLAYER_BIG_BOTTOM,
    SPR_ENEMY_WALKER,
    SPR_ENEMY_HOPPER,
    SPR_BRICK,
    SPR_USED_BLOCK,
    SPR_STAR_SMILE,
    SPR_GROW_POWER,
    SPR_LIGHT_POWER,
    SPR_BOLT,
    SPR_GROUND,
    SPR_SPIKES,
    SPR_ROPE_SEGMENT,
    SPR_ROPE_KNOT,
    SPR_TITLE_ROCKET
};

extern const unsigned char sprite_tiles[];
extern const unsigned short sprite_pal[];

#define SPRITE_TILES_LEN 2048
#define SPRITE_PAL_LEN 32

#endif
