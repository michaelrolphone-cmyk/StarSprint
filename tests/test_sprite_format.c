#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "../src/sprite_format.h"
#include "../src/assets.h"

static void test_single_tile_relayout(void) {
    uint8_t src[32];
    uint8_t dst[32];
    uint8_t expected[32];
    int row;

    for (row = 0; row < 8; row++) {
        src[row * 4 + 0] = (uint8_t)(0x10 + row);
        src[row * 4 + 1] = (uint8_t)(0x20 + row);
        src[row * 4 + 2] = (uint8_t)(0x30 + row);
        src[row * 4 + 3] = (uint8_t)(0x40 + row);
    }

    for (row = 0; row < 8; row++) {
        expected[row * 2 + 0] = (uint8_t)(0x10 + row);
        expected[row * 2 + 1] = (uint8_t)(0x20 + row);
        expected[16 + row * 2 + 0] = (uint8_t)(0x30 + row);
        expected[16 + row * 2 + 1] = (uint8_t)(0x40 + row);
    }

    memset(dst, 0, sizeof(dst));
    convert_interleaved_4bpp_to_snes(src, dst, sizeof(src));
    assert(memcmp(dst, expected, sizeof(dst)) == 0);
}

static void test_first_sprite_tile_matches_expected_planes(void) {
    uint8_t converted[32];
    static const uint8_t expected[32] = {
        0x0F, 0x0C, 0x1C, 0x1C, 0x1F, 0x0C, 0x3F, 0x00,
        0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x02,
        0x0C, 0x00, 0x0F, 0x00, 0x0C, 0x00, 0x1B, 0x03,
        0x1F, 0x07, 0x1D, 0x05, 0x1F, 0x07, 0x1B, 0x03
    };

    convert_interleaved_4bpp_to_snes(sprite_tiles, converted, sizeof(converted));
    assert(memcmp(converted, expected, sizeof(converted)) == 0);
}

static void test_sprite_frame_offset_16x16_layout(void) {
    assert(sprite_frame_offset_16x16(0) == 0);
    assert(sprite_frame_offset_16x16(1) == 4);
    assert(sprite_frame_offset_16x16(7) == 28);
    assert(sprite_frame_offset_16x16(8) == 32);
    assert(sprite_frame_offset_16x16(11) == 44);
    assert(sprite_frame_offset_16x16(15) == 60);
}

static void test_sprite_offsets_fit_uploaded_tile_buffer(void) {
    /*
     * 16 frames * 4 tiles/frame = 64 total tiles.
     * The uploaded sprite sheet is 2048 bytes => 64 tiles at 32 bytes/tile.
     */
    uint16_t finalFrameTileOffset = sprite_frame_offset_16x16(15);
    size_t bytesConsumedByFrames = ((size_t)finalFrameTileOffset + 4u) * 32u;
    assert(bytesConsumedByFrames <= SPRITE_TILES_LEN);
}

int main(void) {
    test_single_tile_relayout();
    test_first_sprite_tile_matches_expected_planes();
    test_sprite_frame_offset_16x16_layout();
    test_sprite_offsets_fit_uploaded_tile_buffer();
    return 0;
}
