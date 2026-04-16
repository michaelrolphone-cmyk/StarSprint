#ifndef STARSPRINT_SPRITE_FORMAT_H
#define STARSPRINT_SPRITE_FORMAT_H

#include <stdint.h>
#include <stddef.h>

void convert_interleaved_4bpp_to_snes(const uint8_t *src, uint8_t *dst, size_t byte_len);
uint16_t sprite_frame_offset_16x16(uint8_t frameIndex);

#endif
