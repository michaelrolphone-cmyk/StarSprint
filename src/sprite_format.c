#include "sprite_format.h"

void convert_interleaved_4bpp_to_snes(const uint8_t *src, uint8_t *dst, size_t byte_len) {
    size_t tile;
    for (tile = 0; tile + 31 < byte_len; tile += 32) {
        uint8_t row;
        for (row = 0; row < 8; row++) {
            size_t src_row = tile + (size_t)row * 4;
            size_t dst_row_low = tile + (size_t)row * 2;
            size_t dst_row_high = tile + 16 + (size_t)row * 2;
            dst[dst_row_low] = src[src_row];
            dst[dst_row_low + 1] = src[src_row + 1];
            dst[dst_row_high] = src[src_row + 2];
            dst[dst_row_high + 1] = src[src_row + 3];
        }
    }

    while (tile < byte_len) {
        dst[tile] = src[tile];
        tile++;
    }
}
