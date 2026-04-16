#include <snes.h>
#include "assets.h"
#include "sprite_format.h"
#include "game_logic.h"

extern char tilfont, palfont;

#define LEVEL_W 192
#define LEVEL_H 14
#define TILE_SIZE 16
#define SCREEN_W 256
#define SCREEN_H 224
#define TEXT_ROWS 28

#define WORLD_COUNT 3
#define LEVELS_PER_WORLD 4
#define TOTAL_LEVELS (WORLD_COUNT * LEVELS_PER_WORLD)

#define MAX_ENEMIES 24
#define MAX_STARS 192
#define MAX_POWERUPS 8
#define MAX_BOLTS 6
#define MAX_ROPES 4

#define PLAYER_W 10
#define PLAYER_SMALL_H 14
#define PLAYER_BIG_H 28

#define ENEMY_W 14
#define ENEMY_H 14

#define POWER_W 14
#define POWER_H 14

#define BOLT_W 10
#define BOLT_H 10

#define SPEED_WALK 4
#define SPEED_RUN 6
#define SPEED_SUPER 12
#define JUMP_VELOCITY -12
#define GRAVITY 1
#define MAX_FALL 12
#define ROPE_PHASE_COUNT 15
#define MAX_PLAY_SUBSTEPS 2

#define POINT_STAR 100
#define POINT_ENEMY 200
#define POINT_BLOCK 50
#define SUPER_FRAMES_PER_MINUTE 3600
#define LEVEL_GOAL_X ((LEVEL_W * TILE_SIZE) - 40)

#define BLANK_LINE "                                "

enum TileType {
    TILE_EMPTY = 0,
    TILE_GROUND,
    TILE_BRICK,
    TILE_USED,
    TILE_POWER_GROW,
    TILE_POWER_LIGHT,
    TILE_SPIKES
};

enum PowerType {
    POWER_GROW = 1,
    POWER_LIGHTNING = 2
};

enum EnemyType {
    ENEMY_WALK = 0,
    ENEMY_HOP = 1
};

enum GameState {
    STATE_TITLE = 0,
    STATE_WORLD_MAP = 1,
    STATE_PLAY = 2,
    STATE_LEVEL_CLEAR = 3,
    STATE_ALL_CLEAR = 4
};

typedef struct {
    s16 x;
    s16 y;
    s16 vx;
    s16 vy;
    u8 onGround;
    u8 facingLeft;
    u8 big;
    u8 lightning;
    u8 smash;
    u8 invuln;
    u8 cooldown;
    u8 onRope;
    u8 ropeIndex;
    u8 ropeGrabLock;
} Player;

typedef struct {
    u8 active;
    u8 kind;
    s16 x;
    s16 y;
    s16 vx;
    s16 vy;
    u8 onGround;
    u8 timer;
} Enemy;

typedef struct {
    u8 active;
    s16 x;
    s16 y;
} StarItem;

typedef struct {
    u8 active;
    u8 type;
    s16 x;
    s16 y;
    s16 vx;
    s16 vy;
    u8 onGround;
} Powerup;

typedef struct {
    u8 active;
    s16 x;
    s16 y;
    s16 vx;
    s16 vy;
    u8 bounces;
} Bolt;

typedef struct {
    u8 active;
    s16 anchorX;
    s16 anchorY;
    s16 x;
    s16 y;
    s16 prevX;
    s16 prevY;
    u8 phase;
    s8 dir;
} Rope;

static u8 levelMap[LEVEL_H][LEVEL_W];
static Player player;
static Enemy enemies[MAX_ENEMIES];
static StarItem stars[MAX_STARS];
static Powerup powerups[MAX_POWERUPS];
static Bolt bolts[MAX_BOLTS];
static Rope ropes[MAX_ROPES];

static u16 score = 0;
static u8 starsTowardMinute = 0;
static u16 superReserveFrames = 0;
static u8 superActive = 0;
static u16 cameraX = 0;
static u16 pad0 = 0;
static u16 padPrev = 0;
static u8 spriteCount = 0;
static u8 spriteTilesVram[SPRITE_TILES_LEN];

static u8 currentLevel = 0;
static u8 selectedLevel = 0;
static u8 highestUnlocked = 0;
static u16 completedBits = 0;
static u8 gameState = STATE_TITLE;
static u8 lastState = 255;

static const char *worldNames[WORLD_COUNT] = {
    "MEADOW",
    "RUINS",
    "STORM"
};

static const s16 ropeSwingX[ROPE_PHASE_COUNT] = { -44, -38, -31, -24, -17, -11, -6, 0, 6, 11, 17, 24, 31, 38, 44 };
static const s16 ropeSwingY[ROPE_PHASE_COUNT] = { 22, 16, 11, 7, 4, 2, 1, 0, 1, 2, 4, 7, 11, 16, 22 };
static const u16 uiTextPal[16] = {
    0x0000, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF
};

static u16 frame_offset(u8 frameIndex) {
    return ((frameIndex >> 3) * 32) + ((frameIndex & 7) * 2);
}

static void clear_text_screen(void) {
    u8 y;
    for (y = 0; y < TEXT_ROWS; y++) {
        consoleDrawText(0, y, BLANK_LINE);
    }
}

static void clear_world(void) {
    u16 x, y;
    for (y = 0; y < LEVEL_H; y++) {
        for (x = 0; x < LEVEL_W; x++) {
            levelMap[y][x] = TILE_EMPTY;
        }
    }
}

static void reset_entities(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
    for (i = 0; i < MAX_STARS; i++) stars[i].active = 0;
    for (i = 0; i < MAX_POWERUPS; i++) powerups[i].active = 0;
    for (i = 0; i < MAX_BOLTS; i++) bolts[i].active = 0;
    for (i = 0; i < MAX_ROPES; i++) ropes[i].active = 0;
}

static void set_tile(s16 tx, s16 ty, u8 tile) {
    if (tx < 0 || tx >= LEVEL_W || ty < 0 || ty >= LEVEL_H) return;
    levelMap[(u8)ty][(u16)tx] = tile;
}

static void set_span(s16 tx0, s16 tx1, s16 ty, u8 tile) {
    s16 x;
    if (ty < 0 || ty >= LEVEL_H) return;
    if (tx0 > tx1) {
        s16 t = tx0;
        tx0 = tx1;
        tx1 = t;
    }
    if (tx0 < 0) tx0 = 0;
    if (tx1 >= LEVEL_W) tx1 = LEVEL_W - 1;
    for (x = tx0; x <= tx1; x++) {
        levelMap[(u8)ty][(u16)x] = tile;
    }
}

static void set_column(s16 tx, s16 ty0, s16 ty1, u8 tile) {
    s16 y;
    if (tx < 0 || tx >= LEVEL_W) return;
    if (ty0 > ty1) {
        s16 t = ty0;
        ty0 = ty1;
        ty1 = t;
    }
    if (ty0 < 0) ty0 = 0;
    if (ty1 >= LEVEL_H) ty1 = LEVEL_H - 1;
    for (y = ty0; y <= ty1; y++) {
        levelMap[(u8)y][(u16)tx] = tile;
    }
}

static void add_enemy(u8 kind, s16 tx, s16 ty, s16 dir) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = 1;
            enemies[i].kind = kind;
            enemies[i].x = tx * TILE_SIZE + 1;
            enemies[i].y = ty * TILE_SIZE + 1;
            enemies[i].vx = dir;
            enemies[i].vy = 0;
            enemies[i].onGround = 0;
            enemies[i].timer = 0;
            return;
        }
    }
}

static void add_star(s16 tx, s16 ty) {
    u8 i;
    for (i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active) {
            stars[i].active = 1;
            stars[i].x = tx * TILE_SIZE;
            stars[i].y = ty * TILE_SIZE;
            return;
        }
    }
}

static void add_star_line(s16 tx0, s16 tx1, s16 ty) {
    s16 x;
    if (tx0 > tx1) {
        s16 t = tx0;
        tx0 = tx1;
        tx1 = t;
    }
    for (x = tx0; x <= tx1; x++) add_star(x, ty);
}

static void add_star_arch(s16 tx0, s16 tx1, s16 ty) {
    s16 mid;
    if (tx0 > tx1) {
        s16 t = tx0;
        tx0 = tx1;
        tx1 = t;
    }
    mid = (tx0 + tx1) >> 1;
    add_star_line(tx0, tx1, ty);
    add_star(mid - 1, ty - 1);
    add_star(mid, ty - 2);
    add_star(mid + 1, ty - 1);
}

static void spawn_powerup(u8 type, s16 tx, s16 ty) {
    u8 i;
    for (i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) {
            powerups[i].active = 1;
            powerups[i].type = type;
            powerups[i].x = tx * TILE_SIZE;
            powerups[i].y = ty * TILE_SIZE - 2;
            powerups[i].vx = 1;
            powerups[i].vy = -3;
            powerups[i].onGround = 0;
            return;
        }
    }
}


static void rope_update_position(Rope *rope) {
    rope->x = rope->anchorX + ropeSwingX[rope->phase];
    rope->y = rope->anchorY + 54 + ropeSwingY[rope->phase];
}

static void add_rope(s16 anchorX, s16 anchorY, u8 phase, s8 dir) {
    u8 i;
    for (i = 0; i < MAX_ROPES; i++) {
        if (!ropes[i].active) {
            ropes[i].active = 1;
            ropes[i].anchorX = anchorX;
            ropes[i].anchorY = anchorY;
            ropes[i].phase = phase % ROPE_PHASE_COUNT;
            ropes[i].dir = (dir >= 0) ? 1 : -1;
            rope_update_position(&ropes[i]);
            ropes[i].prevX = ropes[i].x;
            ropes[i].prevY = ropes[i].y;
            return;
        }
    }
}

static void build_base_floor(void) {
    s16 x;
    for (x = 0; x < LEVEL_W; x++) {
        set_tile(x, 12, TILE_GROUND);
        set_tile(x, 13, TILE_GROUND);
    }
}

static void carve_gap(s16 tx0, s16 width) {
    s16 x;
    for (x = 0; x < width; x++) {
        set_tile(tx0 + x, 12, TILE_EMPTY);
        set_tile(tx0 + x, 13, TILE_EMPTY);
    }
}

static void add_spike_strip(s16 tx0, s16 tx1) {
    set_span(tx0, tx1, 12, TILE_SPIKES);
    set_span(tx0, tx1, 13, TILE_GROUND);
}

static void add_ground_platform(s16 tx0, s16 tx1, s16 ty) {
    set_span(tx0, tx1, ty, TILE_GROUND);
}

static void add_brick_strip(s16 tx0, s16 tx1, s16 ty) {
    set_span(tx0, tx1, ty, TILE_BRICK);
}

static void add_hidden_power(s16 tx, s16 ty, u8 type) {
    set_tile(tx, ty, (type == POWER_GROW) ? TILE_POWER_GROW : TILE_POWER_LIGHT);
}

static void add_stairs(s16 tx, s16 steps, s16 dir) {
    s16 i;
    for (i = 0; i < steps; i++) {
        set_column(tx + (i * dir), 12 - i, 13, TILE_GROUND);
    }
}

static void add_zone_enemy_line(s16 tx0, s16 tx1, u8 count, u8 startKind) {
    u8 i;
    s16 span = tx1 - tx0;
    if (count == 0 || span <= 2) return;
    for (i = 0; i < count; i++) {
        s16 tx = tx0 + 1 + ((span - 2) * (i + 1)) / (count + 1);
        add_enemy(((i + startKind) & 1) ? ENEMY_HOP : ENEMY_WALK, tx, 11, (i & 1) ? -1 : 1);
    }
}

static void build_level(u8 levelIndex) {
    u8 world;
    u8 stage;
    u8 i;
    s16 gapBase;
    s16 platY;
    s16 blockBase;
    s16 hazardBase;
    s16 tx;

    clear_world();
    reset_entities();
    build_base_floor();

    world = levelIndex / LEVELS_PER_WORLD;
    stage = levelIndex % LEVELS_PER_WORLD;

    for (i = 0; i < (u8)(3 + world); i++) {
        s16 width = 2 + ((world + stage + i) & 1) + ((world == 2 && (i & 1)) ? 1 : 0);
        gapBase = 26 + i * 34 + ((stage * 7 + world * 9 + i * 5) % 10);
        if (gapBase + width >= LEVEL_W - 22) continue;

        carve_gap(gapBase, width);
        platY = 9 - ((i + stage + world) % 3);
        add_ground_platform(gapBase - 2, gapBase + width + 1, platY);
        add_star_arch(gapBase - 1, gapBase + width + 1, platY - 1);

        if (((i + stage + world) & 1) == 0) {
            add_enemy((i & 1) ? ENEMY_HOP : ENEMY_WALK, gapBase + width + 2, 11, -1);
        }
    }

    for (i = 0; i < (u8)(2 + world); i++) {
        hazardBase = 40 + i * 42 + ((stage * 5 + i * 7) % 10);
        if (hazardBase >= LEVEL_W - 24) continue;
        add_spike_strip(hazardBase, hazardBase + 2 + ((stage + i) & 1));
        add_star_line(hazardBase, hazardBase + 3, 10 - (i & 1));
    }

    for (i = 0; i < (u8)(5 + world); i++) {
        s16 len = 3 + ((world + stage + i) & 1);
        blockBase = 10 + i * 28 + ((stage * 3 + world * 5 + i * 7) % 9);
        platY = 9 - ((i + world) % 4);
        if (blockBase + len >= LEVEL_W - 10) continue;

        if ((i + world) & 1) {
            add_brick_strip(blockBase, blockBase + len, platY);
            add_star_line(blockBase, blockBase + len, platY - 1);
        } else {
            add_ground_platform(blockBase, blockBase + len, platY);
            add_star_arch(blockBase, blockBase + len, platY - 1);
        }

        if (i == 1 || (i == (u8)(3 + world) && world > 0)) {
            add_hidden_power(blockBase + (len >> 1), platY, ((i + stage) & 1) ? POWER_LIGHTNING : POWER_GROW);
        }
    }

    for (i = 0; i < (u8)(3 + world); i++) {
        tx = 18 + i * 44 + ((world * 5 + stage * 9 + i * 3) % 10);
        if (tx >= LEVEL_W - 16) continue;
        add_ground_platform(tx, tx + 2, 11 - (i & 1));
        add_star_line(tx, tx + 2, 9 - (i & 1));
        if ((i + stage) & 1) add_enemy(ENEMY_WALK, tx + 1, 10 - (i & 1), -1);
    }

    if (world == 0) {
        add_brick_strip(14, 18, 9);
        add_hidden_power(16, 9, POWER_GROW);
        add_star_line(14, 18, 8);
        add_zone_enemy_line(20, 176, 5 + stage, stage);
    } else if (world == 1) {
        add_stairs(52, 4, 1);
        add_stairs(98, 5, -1);
        add_stairs(144, 4, 1);
        add_brick_strip(70, 75, 7);
        add_hidden_power(73, 7, POWER_LIGHTNING);
        add_zone_enemy_line(18, 176, 6 + stage, world + stage);
    } else {
        add_stairs(42, 5, 1);
        add_stairs(88, 6, -1);
        add_stairs(136, 5, 1);
        add_brick_strip(120, 126, 6);
        add_hidden_power(123, 6, POWER_LIGHTNING);
        add_spike_strip(156, 161);
        add_zone_enemy_line(16, 178, 7 + stage, world + stage);
    }

    add_stairs(LEVEL_W - 18, 5, 1);
    add_stairs(LEVEL_W - 8, 4, -1);
    add_star_line(LEVEL_W - 18, LEVEL_W - 10, 6 - world);

    if (levelIndex == 0) {
        set_span(43, 47, 8, TILE_EMPTY);
        add_rope(44 * TILE_SIZE, 44, 1, 1);
        add_star_arch(38, 47, 6);
        add_ground_platform(38, 39, 10);
        add_ground_platform(44, 46, 10);
    }
}

static void reset_player_position(void) {
    player.x = 24;
    player.y = 9 * TILE_SIZE;
    player.vx = 0;
    player.vy = 0;
    player.onGround = 0;
    player.facingLeft = 0;
    player.smash = 0;
    player.invuln = 0;
    player.cooldown = 0;
    player.onRope = 0;
    player.ropeIndex = 255;
    player.ropeGrabLock = 0;
    cameraX = 0;
}

static void begin_level(u8 levelIndex) {
    currentLevel = levelIndex;
    superActive = 0;
    player.big = 0;
    player.lightning = 0;
    build_level(currentLevel);
    reset_player_position();
    gameState = STATE_PLAY;
}

static s16 player_height(void) {
    return player.big ? PLAYER_BIG_H : PLAYER_SMALL_H;
}

static u8 is_level_completed(u8 levelIndex) {
    return (completedBits & (1u << levelIndex)) ? 1 : 0;
}

static u8 is_level_unlocked(u8 levelIndex) {
    return (levelIndex <= highestUnlocked) ? 1 : 0;
}

static u8 completed_count(void) {
    u16 bits = completedBits;
    u8 count = 0;
    while (bits) {
        if (bits & 1u) count++;
        bits >>= 1;
    }
    return count;
}

static u8 is_solid(u8 tile) {
    return (tile == TILE_GROUND || tile == TILE_BRICK || tile == TILE_USED || tile == TILE_POWER_GROW || tile == TILE_POWER_LIGHT || tile == TILE_SPIKES);
}

static u8 is_breakable(u8 tile) {
    return (tile == TILE_BRICK);
}

static u8 is_question(u8 tile) {
    return (tile == TILE_POWER_GROW || tile == TILE_POWER_LIGHT);
}

static u8 tile_at(s16 tx, s16 ty) {
    if (tx < 0 || tx >= LEVEL_W) return TILE_GROUND;
    if (ty < 0) return TILE_GROUND;
    if (ty >= LEVEL_H) return TILE_EMPTY;
    return levelMap[(u8)ty][(u16)tx];
}

static u8 rect_collide_tile(s16 x, s16 y, s16 w, s16 h, s16 *hitTx, s16 *hitTy) {
    s16 left = x / TILE_SIZE;
    s16 right = (x + w - 1) / TILE_SIZE;
    s16 top = y / TILE_SIZE;
    s16 bottom = (y + h - 1) / TILE_SIZE;
    s16 tx, ty;
    for (ty = top; ty <= bottom; ty++) {
        for (tx = left; tx <= right; tx++) {
            if (is_solid(tile_at(tx, ty))) {
                *hitTx = tx;
                *hitTy = ty;
                return 1;
            }
        }
    }
    return 0;
}

static void break_tile(s16 tx, s16 ty) {
    if (tx >= 0 && tx < LEVEL_W && ty >= 0 && ty < LEVEL_H) {
        levelMap[(u8)ty][(u8)tx] = TILE_EMPTY;
        score += POINT_BLOCK;
    }
}

static void hit_block_from_below(s16 tx, s16 ty) {
    u8 tile = tile_at(tx, ty);
    if (is_breakable(tile)) {
        break_tile(tx, ty);
        return;
    }
    if (tile == TILE_POWER_GROW) {
        levelMap[(u8)ty][(u8)tx] = TILE_USED;
        spawn_powerup(POWER_GROW, tx, ty);
        score += POINT_BLOCK;
        return;
    }
    if (tile == TILE_POWER_LIGHT) {
        levelMap[(u8)ty][(u8)tx] = TILE_USED;
        spawn_powerup(POWER_LIGHTNING, tx, ty);
        score += POINT_BLOCK;
        return;
    }
}

static u8 overlap(s16 ax, s16 ay, s16 aw, s16 ah, s16 bx, s16 by, s16 bw, s16 bh) {
    if (ax + aw <= bx) return 0;
    if (bx + bw <= ax) return 0;
    if (ay + ah <= by) return 0;
    if (by + bh <= ay) return 0;
    return 1;
}

static void restart_current_level(void) {
    superActive = 0;
    player.big = 0;
    player.lightning = 0;
    build_level(currentLevel);
    reset_player_position();
}

static void player_take_hit(void) {
    if (player.invuln) return;

    if (player.lightning) {
        player.lightning = 0;
        player.invuln = 60;
        return;
    }
    if (player.big) {
        player.big = 0;
        player.invuln = 60;
        return;
    }

    restart_current_level();
}

static void activate_star_reward(void) {
    while (starsTowardMinute >= 60) {
        starsTowardMinute -= 60;
        superReserveFrames += SUPER_FRAMES_PER_MINUTE;
    }
}

static void update_ropes(void) {
    u8 i;
    for (i = 0; i < MAX_ROPES; i++) {
        if (!ropes[i].active) continue;
        ropes[i].prevX = ropes[i].x;
        ropes[i].prevY = ropes[i].y;
        if (ropes[i].dir > 0) {
            if (ropes[i].phase + 1 >= ROPE_PHASE_COUNT) {
                ropes[i].dir = -1;
                ropes[i].phase--;
            } else {
                ropes[i].phase++;
            }
        } else {
            if (ropes[i].phase == 0) {
                ropes[i].dir = 1;
                ropes[i].phase++;
            } else {
                ropes[i].phase--;
            }
        }
        rope_update_position(&ropes[i]);
    }
}

static void try_grab_rope(void) {
    u8 i;
    s16 h = player_height();
    if (!should_attempt_rope_grab(player.onRope, player.onGround, player.ropeGrabLock)) return;
    for (i = 0; i < MAX_ROPES; i++) {
        if (!ropes[i].active) continue;
        if (overlap(player.x, player.y, PLAYER_W, h, ropes[i].x - 8, ropes[i].y - 8, 16, 16)) {
            player.onRope = 1;
            player.ropeIndex = i;
            player.vx = 0;
            player.vy = 0;
            player.x = ropes[i].x - (PLAYER_W >> 1);
            player.y = ropes[i].y - h + 4;
            return;
        }
    }
}

static void fire_bolt(void) {
    u8 i;
    if (!player.lightning || player.cooldown) return;
    for (i = 0; i < MAX_BOLTS; i++) {
        if (!bolts[i].active) {
            bolts[i].active = 1;
            bolts[i].x = player.x + (player.facingLeft ? -4 : PLAYER_W + 2);
            bolts[i].y = player.y + (player.big ? 10 : 4);
            bolts[i].vx = player.facingLeft ? -6 : 6;
            bolts[i].vy = -2;
            bolts[i].bounces = 3;
            player.cooldown = 16;
            return;
        }
    }
}

static void update_player_input(void) {
    u8 jumpPressed = ((pad0 & KEY_B) && !(padPrev & KEY_B));
    u8 yHeld = (pad0 & KEY_Y) ? 1 : 0;
    u8 yPressed = ((pad0 & KEY_Y) && !(padPrev & KEY_Y));
    s16 maxSpeed;

    superActive = (yHeld && superReserveFrames > 0) ? 1 : 0;
    maxSpeed = superActive ? SPEED_SUPER : (yHeld ? SPEED_RUN : SPEED_WALK);

    if (player.onRope) {
        Rope *rope = &ropes[player.ropeIndex];
        player.facingLeft = (rope->x < rope->prevX) ? 1 : 0;
        player.smash = 0;
        if (jumpPressed) {
            s16 dx = rope->x - rope->prevX;
            s16 dy = rope->y - rope->prevY;
            player.onRope = 0;
            player.ropeIndex = 255;
            player.ropeGrabLock = 10;
            player.vx = dx * 2;
            if (player.vx == 0) player.vx = rope->dir * 4;
            player.vy = (dy * 2) - 3;
            if (player.vy < -10) player.vy = -10;
            if (player.vy > 2) player.vy = 2;
        }
        if (yPressed && player.lightning) fire_bolt();
        return;
    }

    if (pad0 & KEY_LEFT) {
        player.vx--;
        if (player.vx < -maxSpeed) player.vx = -maxSpeed;
        player.facingLeft = 1;
    } else if (pad0 & KEY_RIGHT) {
        player.vx++;
        if (player.vx > maxSpeed) player.vx = maxSpeed;
        player.facingLeft = 0;
    } else {
        if (player.vx > 0) player.vx--;
        if (player.vx < 0) player.vx++;
    }

    if (jumpPressed && player.onGround) {
        player.vy = JUMP_VELOCITY;
        player.onGround = 0;
    }

    player.smash = (!player.onGround && (pad0 & KEY_DOWN) && player.vy > 0);

    if (yPressed && player.lightning) {
        fire_bolt();
    }
}

static u8 try_player_tunnel_step(s16 step, s16 h) {
    static const s8 offsets[] = { -2, -1, 1, 2, -3, 3 };
    u8 i;
    s16 hitTx = 0, hitTy = 0;

    if (player.big) return 0;

    for (i = 0; i < (u8)(sizeof(offsets) / sizeof(offsets[0])); i++) {
        s16 ny = player.y + offsets[i];
        if (ny < 0) continue;
        if (ny > (LEVEL_H * TILE_SIZE) - h) continue;
        if (rect_collide_tile(player.x, ny, PLAYER_W, h, &hitTx, &hitTy)) continue;
        if (rect_collide_tile(player.x + step, ny, PLAYER_W, h, &hitTx, &hitTy)) continue;
        player.y = ny;
        player.x += step;
        return 1;
    }

    return 0;
}

static void move_player(void) {
    s16 step;
    s16 hitTx = 0, hitTy = 0;
    s16 h = player_height();

    if (player.onRope) {
        if (player.ropeIndex >= MAX_ROPES || !ropes[player.ropeIndex].active) {
            player.onRope = 0;
            player.ropeIndex = 255;
        } else {
            player.onGround = 0;
            player.vx = 0;
            player.vy = 0;
            player.x = ropes[player.ropeIndex].x - (PLAYER_W >> 1);
            player.y = ropes[player.ropeIndex].y - h + 4;
            return;
        }
    }

    player.onGround = 0;

    if (player.vy < MAX_FALL) player.vy += GRAVITY;
    if (player.vy > MAX_FALL) player.vy = MAX_FALL;

    if (player.invuln) player.invuln--;
    if (player.cooldown) player.cooldown--;
    if (player.ropeGrabLock) player.ropeGrabLock--;

    {
        s16 remainingX = player.vx;
        if (remainingX != 0) {
            step = (remainingX < 0) ? -1 : 1;
            while (remainingX != 0) {
                if (!rect_collide_tile(player.x + step, player.y, PLAYER_W, h, &hitTx, &hitTy)) {
                    player.x += step;
                    remainingX -= step;
                } else if (try_player_tunnel_step(step, h)) {
                    remainingX -= step;
                } else {
                    player.vx = 0;
                    break;
                }
            }
        }
    }

    if (player.y > (LEVEL_H * TILE_SIZE) + 40) {
        player_take_hit();
        return;
    }

    {
        s16 remainingY = player.vy;
        if (remainingY != 0) {
            step = (remainingY < 0) ? -1 : 1;
            while (remainingY != 0) {
                if (!rect_collide_tile(player.x, player.y + step, PLAYER_W, h, &hitTx, &hitTy)) {
                    player.y += step;
                    remainingY -= step;
                } else {
                    if (step < 0) {
                        hit_block_from_below(hitTx, hitTy);
                        player.vy = 0;
                    } else {
                        u8 tile = tile_at(hitTx, hitTy);
                        if (player.smash && is_breakable(tile)) {
                            break_tile(hitTx, hitTy);
                            player.y += 1;
                            if (player.vy < 6) player.vy = 6;
                        } else {
                            if (tile == TILE_SPIKES) {
                                player_take_hit();
                                return;
                            }
                            player.onGround = 1;
                            player.vy = 0;
                        }
                    }
                    break;
                }
            }
        }
    }

    try_grab_rope();

    if (player.x < 0) player.x = 0;
    if (player.x > (LEVEL_W * TILE_SIZE) - PLAYER_W) player.x = (LEVEL_W * TILE_SIZE) - PLAYER_W;

    if (player.y + h >= SCREEN_H && player.vy == 0) player.onGround = 1;
}

static u8 enemy_floor_ahead(const Enemy *enemy) {
    s16 lookX = enemy->x + ((enemy->vx < 0) ? -1 : ENEMY_W);
    s16 lookY = enemy->y + ENEMY_H + 1;
    u8 tile = tile_at(lookX / TILE_SIZE, lookY / TILE_SIZE);
    return (tile != TILE_EMPTY && tile != TILE_SPIKES);
}

static void update_enemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        s16 hitTx = 0, hitTy = 0;
        s16 step;
        if (!enemies[i].active) continue;

        enemies[i].timer++;
        if (enemies[i].onGround && !enemy_floor_ahead(&enemies[i])) {
            enemies[i].vx = -enemies[i].vx;
        }
        if (enemies[i].kind == ENEMY_HOP && enemies[i].onGround && (enemies[i].timer & 31) == 0) {
            enemies[i].vy = -7;
            enemies[i].onGround = 0;
        }

        if (enemies[i].vy < MAX_FALL) enemies[i].vy += GRAVITY;
        if (enemies[i].vy > MAX_FALL) enemies[i].vy = MAX_FALL;

        {
            s16 remainingX = enemies[i].vx;
            if (remainingX != 0) {
                step = (remainingX < 0) ? -1 : 1;
                while (remainingX != 0) {
                    if (!rect_collide_tile(enemies[i].x + step, enemies[i].y, ENEMY_W, ENEMY_H, &hitTx, &hitTy)) {
                        enemies[i].x += step;
                        remainingX -= step;
                    } else {
                        enemies[i].vx = -enemies[i].vx;
                        break;
                    }
                }
            }
        }

        {
            s16 remainingY = enemies[i].vy;
            enemies[i].onGround = 0;
            if (remainingY != 0) {
                step = (remainingY < 0) ? -1 : 1;
                while (remainingY != 0) {
                    if (!rect_collide_tile(enemies[i].x, enemies[i].y + step, ENEMY_W, ENEMY_H, &hitTx, &hitTy)) {
                        enemies[i].y += step;
                        remainingY -= step;
                    } else {
                        if (step > 0) {
                            enemies[i].onGround = 1;
                            if (tile_at(hitTx, hitTy) == TILE_SPIKES) {
                                enemies[i].active = 0;
                            }
                        }
                        enemies[i].vy = 0;
                        break;
                    }
                }
            }
        }

        if (enemies[i].active) {
            enemies[i].vx = (enemies[i].vx < 0) ? -1 : 1;
        }
    }
}

static void update_powerups(void) {
    u8 i;
    for (i = 0; i < MAX_POWERUPS; i++) {
        s16 hitTx = 0, hitTy = 0;
        s16 step;
        if (!powerups[i].active) continue;

        if (powerups[i].vy < MAX_FALL) powerups[i].vy += GRAVITY;
        if (powerups[i].vy > MAX_FALL) powerups[i].vy = MAX_FALL;

        {
            s16 remainingX = powerups[i].vx;
            if (remainingX != 0) {
                step = (remainingX < 0) ? -1 : 1;
                while (remainingX != 0) {
                    if (!rect_collide_tile(powerups[i].x + step, powerups[i].y, POWER_W, POWER_H, &hitTx, &hitTy)) {
                        powerups[i].x += step;
                        remainingX -= step;
                    } else {
                        powerups[i].vx = -step;
                        break;
                    }
                }
            }
        }

        {
            s16 remainingY = powerups[i].vy;
            powerups[i].onGround = 0;
            if (remainingY != 0) {
                step = (remainingY < 0) ? -1 : 1;
                while (remainingY != 0) {
                    if (!rect_collide_tile(powerups[i].x, powerups[i].y + step, POWER_W, POWER_H, &hitTx, &hitTy)) {
                        powerups[i].y += step;
                        remainingY -= step;
                    } else {
                        if (step > 0) powerups[i].onGround = 1;
                        powerups[i].vy = 0;
                        break;
                    }
                }
            }
        }

        if (powerups[i].onGround && powerups[i].vx == 0) powerups[i].vx = 1;
    }
}

static void update_bolts(void) {
    u8 i;
    for (i = 0; i < MAX_BOLTS; i++) {
        s16 hitTx = 0, hitTy = 0;
        s16 step;
        if (!bolts[i].active) continue;

        if (bolts[i].vy < MAX_FALL) bolts[i].vy += 1;
        if (bolts[i].vy > MAX_FALL) bolts[i].vy = MAX_FALL;

        {
            s16 remainingX = bolts[i].vx;
            if (remainingX != 0) {
                step = (remainingX < 0) ? -1 : 1;
                while (remainingX != 0) {
                    if (!rect_collide_tile(bolts[i].x + step, bolts[i].y, BOLT_W, BOLT_H, &hitTx, &hitTy)) {
                        bolts[i].x += step;
                        remainingX -= step;
                    } else {
                        bolts[i].vx = -step * 6;
                        if (bolts[i].bounces) bolts[i].bounces--;
                        else bolts[i].active = 0;
                        break;
                    }
                }
            }
        }

        if (!bolts[i].active) continue;

        {
            s16 remainingY = bolts[i].vy;
            if (remainingY != 0) {
                step = (remainingY < 0) ? -1 : 1;
                while (remainingY != 0) {
                    if (!rect_collide_tile(bolts[i].x, bolts[i].y + step, BOLT_W, BOLT_H, &hitTx, &hitTy)) {
                        bolts[i].y += step;
                        remainingY -= step;
                    } else {
                        if (step > 0) {
                            bolts[i].vy = -6;
                            if (bolts[i].bounces) bolts[i].bounces--;
                            else bolts[i].active = 0;
                        } else {
                            bolts[i].vy = 2;
                        }
                        break;
                    }
                }
            }
        }
    }
}

static u8 count_active_bolts(void) {
    u8 i;
    u8 active = 0;
    for (i = 0; i < MAX_BOLTS; i++) {
        if (bolts[i].active) active++;
    }
    return active;
}

static void handle_pickups_and_hits(void) {
    u8 i;

    for (i = 0; i < MAX_STARS; i++) {
        if (stars[i].active && overlap(player.x, player.y, PLAYER_W, player_height(), stars[i].x + 2, stars[i].y + 2, 12, 12)) {
            stars[i].active = 0;
            score += POINT_STAR;
            starsTowardMinute++;
            activate_star_reward();
        }
    }

    for (i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active && overlap(player.x, player.y, PLAYER_W, player_height(), powerups[i].x, powerups[i].y, POWER_W, POWER_H)) {
            if (powerups[i].type == POWER_GROW) {
                player.big = 1;
            } else if (powerups[i].type == POWER_LIGHTNING) {
                player.big = 1;
                player.lightning = 1;
            }
            powerups[i].active = 0;
            score += 250;
        }
    }

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        if (overlap(player.x, player.y, PLAYER_W, player_height(), enemies[i].x, enemies[i].y, ENEMY_W, ENEMY_H)) {
            if (player.vy > 0 && (player.y + player_height()) <= (enemies[i].y + 8)) {
                enemies[i].active = 0;
                score += POINT_ENEMY;
                player.vy = -7;
            } else {
                player_take_hit();
            }
        }
    }

    for (i = 0; i < MAX_BOLTS; i++) {
        u8 j;
        if (!bolts[i].active) continue;
        for (j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].active && overlap(bolts[i].x, bolts[i].y, BOLT_W, BOLT_H, enemies[j].x, enemies[j].y, ENEMY_W, ENEMY_H)) {
                enemies[j].active = 0;
                bolts[i].active = 0;
                score += POINT_ENEMY;
                break;
            }
        }
    }

    if (player.x >= LEVEL_GOAL_X) {
        gameState = STATE_LEVEL_CLEAR;
    }
}

static void update_camera(void) {
    s16 target = player.x - 96;
    s16 maxCamera = (LEVEL_W * TILE_SIZE) - SCREEN_W;
    if (target < 0) target = 0;
    if (target > maxCamera) target = maxCamera;
    cameraX = (u16)target;
}

static void sprite_begin(void) {
    spriteCount = 0;
}

static void sprite_emit(u8 frame, s16 sx, s16 sy, u8 hflip, u8 pal) {
    u16 id;
    if (spriteCount >= 128) return;
    if (sx <= -16 || sx >= SCREEN_W || sy <= -16 || sy >= SCREEN_H) return;
    id = (u16)spriteCount * 4;
    oamSet(id, (u16)sx, (u16)sy, 3, hflip, 0, frame_offset(frame), pal);
    oamSetEx(id, OBJ_SMALL, OBJ_SHOW);
    spriteCount++;
}

static void sprite_end(void) {
    u16 id;
    while (spriteCount < 128) {
        id = (u16)spriteCount * 4;
        oamSetVisible(id, OBJ_HIDE);
        spriteCount++;
    }
}

static void draw_world(void) {
    s16 tx0 = cameraX / TILE_SIZE;
    s16 tx1 = tx0 + (SCREEN_W / TILE_SIZE) + 2;
    s16 tx, ty;
    u8 tile;
    if (tx1 >= LEVEL_W) tx1 = LEVEL_W - 1;

    for (ty = 0; ty < LEVEL_H; ty++) {
        for (tx = tx0; tx <= tx1; tx++) {
            tile = tile_at(tx, ty);
            if (tile == TILE_EMPTY) continue;

            if (is_question(tile)) tile = TILE_BRICK;

            switch (tile) {
                case TILE_GROUND:
                    sprite_emit(SPR_GROUND, tx * TILE_SIZE - cameraX, ty * TILE_SIZE, 0, 0);
                    break;
                case TILE_BRICK:
                    sprite_emit(SPR_BRICK, tx * TILE_SIZE - cameraX, ty * TILE_SIZE, 0, 0);
                    break;
                case TILE_USED:
                    sprite_emit(SPR_USED_BLOCK, tx * TILE_SIZE - cameraX, ty * TILE_SIZE, 0, 0);
                    break;
                case TILE_SPIKES:
                    sprite_emit(SPR_SPIKES, tx * TILE_SIZE - cameraX, ty * TILE_SIZE, 0, 0);
                    break;
                default:
                    break;
            }
        }
    }
}

static void draw_stars(void) {
    u8 i;
    for (i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            sprite_emit(SPR_STAR_SMILE, stars[i].x - cameraX, stars[i].y, 0, 0);
        }
    }
}

static void draw_enemies(void) {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            sprite_emit(enemies[i].kind == ENEMY_HOP ? SPR_ENEMY_HOPPER : SPR_ENEMY_WALKER,
                        enemies[i].x - cameraX, enemies[i].y, enemies[i].vx < 0, 0);
        }
    }
}

static void draw_powerups(void) {
    u8 i;
    for (i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active) {
            sprite_emit(powerups[i].type == POWER_GROW ? SPR_GROW_POWER : SPR_LIGHT_POWER,
                        powerups[i].x - cameraX, powerups[i].y, 0, 0);
        }
    }
}

static void draw_bolts(void) {
    u8 i;
    for (i = 0; i < MAX_BOLTS; i++) {
        if (bolts[i].active) {
            sprite_emit(SPR_BOLT, bolts[i].x - cameraX, bolts[i].y, bolts[i].vx < 0, 0);
        }
    }
}

static void draw_ropes(void) {
    u8 i;
    for (i = 0; i < MAX_ROPES; i++) {
        if (ropes[i].active) {
            s16 dx = ropes[i].x - ropes[i].anchorX;
            s16 dy = ropes[i].y - ropes[i].anchorY;
            s16 seg;
            sprite_emit(SPR_ROPE_KNOT, ropes[i].anchorX - cameraX - 4, ropes[i].anchorY - 2, 0, 0);
            for (seg = 0; seg <= 2; seg++) {
                s16 sx = ropes[i].anchorX + (dx * (seg + 1)) / 4 - cameraX;
                s16 sy = ropes[i].anchorY + (dy * (seg + 1)) / 4;
                sprite_emit(SPR_ROPE_SEGMENT, sx - 4, sy, 0, 0);
            }
            sprite_emit(SPR_ROPE_KNOT, ropes[i].x - cameraX - 4, ropes[i].y - 2, 0, 0);
        }
    }
}

static void draw_player(void) {
    s16 sx = player.x - cameraX - 2;
    s16 sy = player.y;
    if (player.big) {
        sprite_emit(SPR_PLAYER_BIG_TOP, sx, sy, player.facingLeft, 0);
        sprite_emit(SPR_PLAYER_BIG_BOTTOM, sx, sy + 16, player.facingLeft, 0);
    } else {
        sprite_emit(SPR_PLAYER_SMALL, sx, sy, player.facingLeft, 0);
    }
}

static void draw_play_hud(void) {
    u16 reserveSeconds = superReserveFrames / 60;
    u8 world = (currentLevel / LEVELS_PER_WORLD) + 1;
    u8 stage = (currentLevel % LEVELS_PER_WORLD) + 1;
    consoleDrawText(0, 0, BLANK_LINE);
    consoleDrawText(0, 1, BLANK_LINE);
    consoleDrawText(0, 2, BLANK_LINE);
    consoleDrawText(0, 3, BLANK_LINE);
    consoleDrawText(0, 4, BLANK_LINE);
    consoleDrawText(0, 26, BLANK_LINE);
    consoleDrawText(1, 0, "WORLD %u-%u  %s", world, stage, worldNames[world - 1]);
    consoleDrawText(1, 1, "SCORE %05u", score);
    consoleDrawText(1, 2, "STARS %02u/60", starsTowardMinute);
    consoleDrawText(1, 3, "BOOST %03us %s", reserveSeconds, superActive ? "ON " : "OFF");
    consoleDrawText(1, 4, "FORM  %s", player.lightning ? "LIGHT" : (player.big ? "BIG  " : "SMALL"));
    consoleDrawText(1, 26, "B JUMP  Y RUN/BOOST/FIRE");
}

static void draw_title_scene(void) {
    s16 x;

    for (x = 0; x < 16; x++) {
        sprite_emit(SPR_GROUND, x * 16, 192, 0, 0);
    }

    sprite_emit(SPR_GROUND, 12, 150, 0, 0);
    sprite_emit(SPR_GROUND, 28, 150, 0, 0);
    sprite_emit(SPR_GROUND, 188, 150, 0, 0);
    sprite_emit(SPR_GROUND, 204, 150, 0, 0);

    sprite_emit(SPR_PLAYER_BIG_TOP, 104, 100, 0, 0);
    sprite_emit(SPR_PLAYER_BIG_BOTTOM, 104, 116, 0, 0);

    sprite_emit(SPR_GROW_POWER, 32, 112, 0, 0);
    sprite_emit(SPR_ENEMY_HOPPER, 68, 156, 0, 0);
    sprite_emit(SPR_ENEMY_WALKER, 182, 146, 1, 0);
    sprite_emit(SPR_TITLE_ROCKET, 208, 28, 0, 0);

    sprite_emit(SPR_STAR_SMILE, 22, 46, 0, 0);
    sprite_emit(SPR_STAR_SMILE, 58, 28, 0, 0);
    sprite_emit(SPR_STAR_SMILE, 152, 34, 0, 0);
    sprite_emit(SPR_STAR_SMILE, 184, 58, 0, 0);

    sprite_emit(SPR_LIGHT_POWER, 24, 20, 0, 0);
    sprite_emit(SPR_LIGHT_POWER, 196, 20, 0, 0);
    sprite_emit(SPR_BRICK, 88, 72, 0, 0);
    sprite_emit(SPR_BRICK, 104, 72, 0, 0);
    sprite_emit(SPR_BRICK, 120, 72, 0, 0);
    sprite_emit(SPR_BRICK, 136, 72, 0, 0);
}

static void draw_title_screen(void) {
    consoleDrawText(6, 2, "STARSPRINT");
    consoleDrawText(4, 4, "COVER-STYLE HERO ADVENTURE");
    consoleDrawText(4, 22, "B JUMPS / RELEASES ROPES");
    consoleDrawText(3, 23, "Y RUNS, USES BOOST, AND FIRES");
    consoleDrawText(4, 24, "DOWN SMASHES BRICKS UNDERFOOT");
    consoleDrawText(4, 25, "COLLECT 60 STARS FOR 1 MIN BOOST");
    consoleDrawText(6, 26, "PRESS START FOR MAP");
}

static void update_title_input(void) {
    u8 startPressed = ((pad0 & KEY_START) && !(padPrev & KEY_START));
    u8 aPressed = ((pad0 & KEY_A) && !(padPrev & KEY_A));
    if (startPressed || aPressed) {
        gameState = STATE_WORLD_MAP;
        selectedLevel = highestUnlocked;
    }
}

static void draw_world_map_scene(void) {
    static const s16 mapX[LEVELS_PER_WORLD] = { 56, 104, 152, 200 };
    static const s16 mapY[WORLD_COUNT] = { 80, 128, 176 };
    u8 row;
    u8 col;

    for (row = 0; row < WORLD_COUNT; row++) {
        for (col = 0; col < LEVELS_PER_WORLD; col++) {
            u8 idx = row * LEVELS_PER_WORLD + col;
            u8 unlocked = is_level_unlocked(idx);
            u8 cleared = is_level_completed(idx);
            s16 x = mapX[col];
            s16 y = mapY[row];

            if (col > 0) {
                sprite_emit(SPR_USED_BLOCK, x - 32, y + 2, 0, 0);
                sprite_emit(SPR_USED_BLOCK, x - 16, y + 2, 0, 0);
            }

            if (unlocked) {
                sprite_emit(cleared ? SPR_STAR_SMILE : SPR_BRICK, x, y, 0, 0);
            } else {
                sprite_emit(SPR_USED_BLOCK, x, y, 0, 0);
            }
        }
    }

    {
        u8 row = selectedLevel / LEVELS_PER_WORLD;
        u8 col = selectedLevel % LEVELS_PER_WORLD;
        sprite_emit(SPR_PLAYER_SMALL, mapX[col] + 2, mapY[row] - 18, 0, 0);
    }
}

static void draw_world_map(void) {
    u8 row;
    u8 col;
    consoleDrawText(2, 1, "STARSPRINT WORLD MAP");
    consoleDrawText(2, 2, "D-PAD MOVE  START/A PLAY");
    consoleDrawText(2, 3, "CLEAR LEVELS TO UNLOCK MORE");
    consoleDrawText(2, 4, "BOOST BANK %03us", (u16)(superReserveFrames / 60));

    for (row = 0; row < WORLD_COUNT; row++) {
        u8 y = 7 + row * 6;
        consoleDrawText(1, y, "WORLD %u %-8s", row + 1, worldNames[row]);
        for (col = 0; col < LEVELS_PER_WORLD; col++) {
            u8 idx = row * LEVELS_PER_WORLD + col;
            u8 x = 8 + col * 6;
            char left = (idx == selectedLevel) ? '<' : '[';
            char right = (idx == selectedLevel) ? '>' : ']';
            char status = is_level_completed(idx) ? 'C' : (is_level_unlocked(idx) ? 'O' : 'X');
            if (col > 0) consoleDrawText(x - 3, y + 1, "---");
            consoleDrawText(x, y + 1, "%c%u%c", left, col + 1, right);
            consoleDrawText(x + 1, y + 2, "%c", status);
        }
    }

    {
        u8 world = (selectedLevel / LEVELS_PER_WORLD) + 1;
        u8 stage = (selectedLevel % LEVELS_PER_WORLD) + 1;
        consoleDrawText(1, 24, "SELECTED %u-%u  %s", world, stage, is_level_completed(selectedLevel) ? "CLEAR" : (is_level_unlocked(selectedLevel) ? "OPEN " : "LOCK "));
        consoleDrawText(1, 25, "SCORE %05u  CLEAR %02u/%02u", score, completed_count(), TOTAL_LEVELS);
        consoleDrawText(1, 26, "C=cleared O=open X=locked");
    }
}

static void draw_level_clear_screen(void) {
    u8 world = (currentLevel / LEVELS_PER_WORLD) + 1;
    u8 stage = (currentLevel % LEVELS_PER_WORLD) + 1;
    consoleDrawText(7, 8, "LEVEL CLEAR!");
    consoleDrawText(8, 10, "WORLD %u-%u", world, stage);
    consoleDrawText(6, 12, "SCORE %05u", score);
    consoleDrawText(4, 14, "PRESS A OR START");
    consoleDrawText(6, 15, "FOR WORLD MAP");
}

static void draw_all_clear_screen(void) {
    consoleDrawText(7, 7, "ALL WORLDS CLEAR!");
    consoleDrawText(7, 10, "FINAL SCORE %05u", score);
    consoleDrawText(3, 13, "YOU CROSSED EVERY WORLD");
    consoleDrawText(4, 15, "PRESS START FOR MAP");
}

static void commit_level_clear(void) {
    completedBits |= (1u << currentLevel);
    if (currentLevel + 1 < TOTAL_LEVELS && highestUnlocked < (u8)(currentLevel + 1)) {
        highestUnlocked = currentLevel + 1;
    }
    selectedLevel = currentLevel;
    if (completedBits == (u16)((1u << TOTAL_LEVELS) - 1u)) {
        gameState = STATE_ALL_CLEAR;
    } else {
        gameState = STATE_WORLD_MAP;
    }
}

static void update_world_map_input(void) {
    u8 row = selectedLevel / LEVELS_PER_WORLD;
    u8 col = selectedLevel % LEVELS_PER_WORLD;
    u8 newIndex = selectedLevel;
    u8 startPressed = ((pad0 & KEY_START) && !(padPrev & KEY_START));
    u8 aPressed = ((pad0 & KEY_A) && !(padPrev & KEY_A));
    u8 leftPressed = ((pad0 & KEY_LEFT) && !(padPrev & KEY_LEFT));
    u8 rightPressed = ((pad0 & KEY_RIGHT) && !(padPrev & KEY_RIGHT));
    u8 upPressed = ((pad0 & KEY_UP) && !(padPrev & KEY_UP));
    u8 downPressed = ((pad0 & KEY_DOWN) && !(padPrev & KEY_DOWN));

    if (leftPressed && col > 0) newIndex = selectedLevel - 1;
    if (rightPressed && col + 1 < LEVELS_PER_WORLD) newIndex = selectedLevel + 1;
    if (upPressed && row > 0) newIndex = selectedLevel - LEVELS_PER_WORLD;
    if (downPressed && row + 1 < WORLD_COUNT) newIndex = selectedLevel + LEVELS_PER_WORLD;

    if (is_level_unlocked(newIndex)) selectedLevel = newIndex;

    if ((startPressed || aPressed) && is_level_unlocked(selectedLevel)) {
        begin_level(selectedLevel);
    }
}

static void update_level_clear_input(void) {
    u8 startPressed = ((pad0 & KEY_START) && !(padPrev & KEY_START));
    u8 aPressed = ((pad0 & KEY_A) && !(padPrev & KEY_A));
    if (startPressed || aPressed) {
        commit_level_clear();
    }
}

static void update_all_clear_input(void) {
    u8 startPressed = ((pad0 & KEY_START) && !(padPrev & KEY_START));
    u8 aPressed = ((pad0 & KEY_A) && !(padPrev & KEY_A));
    if (startPressed || aPressed) {
        selectedLevel = highestUnlocked;
        gameState = STATE_WORLD_MAP;
    }
}

static void set_backdrop_for_state(u8 state) {
    if (state == STATE_TITLE) {
        setPaletteColor(0, RGB5(4, 8, 20));
    } else if (state == STATE_WORLD_MAP) {
        setPaletteColor(0, RGB5(8, 14, 22));
    } else if (state == STATE_LEVEL_CLEAR) {
        setPaletteColor(0, RGB5(14, 9, 20));
    } else if (state == STATE_ALL_CLEAR) {
        setPaletteColor(0, RGB5(20, 12, 6));
    } else {
        setPaletteColor(0, RGB5(18, 24, 31));
    }
}

static void init_video(void) {
    consoleSetTextMapPtr(0x6800);
    consoleSetTextGfxPtr(0x3000);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);
    consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));

    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    convert_interleaved_4bpp_to_snes(sprite_tiles, spriteTilesVram, SPRITE_TILES_LEN);
    oamInitGfxSet((u8 *)spriteTilesVram, SPRITE_TILES_LEN, (u8 *)sprite_pal, SPRITE_PAL_LEN, 0, 0x0000, OBJ_SIZE16_L32);

    bgSetScroll(0, 0, 0);
    set_backdrop_for_state(STATE_TITLE);
    setScreenOn();
}

int main(void) {
    init_video();
    reset_player_position();
    player.big = 0;
    player.lightning = 0;

    while (1) {
        padPrev = pad0;
        pad0 = padsCurrent(0);

        if (gameState != lastState) {
            clear_text_screen();
            set_backdrop_for_state(gameState);
            lastState = gameState;
        }

        if (gameState == STATE_TITLE) {
            consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));
            update_title_input();
            sprite_begin();
            draw_title_scene();
            sprite_end();
            draw_title_screen();
        } else if (gameState == STATE_PLAY) {
            u8 simStep;
            u8 simSteps = 1;
            if (superActive) {
                if (superReserveFrames > 0) superReserveFrames--;
                else superActive = 0;
            }

            update_player_input();
            if (should_run_secondary_substep(player.vx, player.vy, superActive, count_active_bolts())) {
                simSteps = MAX_PLAY_SUBSTEPS;
            }
            for (simStep = 0; simStep < simSteps && gameState == STATE_PLAY; simStep++) {
                update_ropes();
                move_player();
                update_enemies();
                update_powerups();
                update_bolts();
                handle_pickups_and_hits();
            }
            update_camera();

            sprite_begin();
            draw_world();
            draw_stars();
            draw_powerups();
            draw_enemies();
            draw_bolts();
            draw_ropes();
            draw_player();
            sprite_end();
            draw_play_hud();
        } else if (gameState == STATE_WORLD_MAP) {
            consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));
            update_world_map_input();
            sprite_begin();
            draw_world_map_scene();
            sprite_end();
            draw_world_map();
        } else if (gameState == STATE_LEVEL_CLEAR) {
            consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));
            update_level_clear_input();
            sprite_begin();
            sprite_end();
            draw_level_clear_screen();
        } else {
            consoleSetTextPal(0, (u8 *)uiTextPal, sizeof(uiTextPal));
            update_all_clear_input();
            sprite_begin();
            sprite_end();
            draw_all_clear_screen();
        }

        WaitForVBlank();
    }

    return 0;
}
