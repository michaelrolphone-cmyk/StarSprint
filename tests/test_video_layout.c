#include <assert.h>
#include <stdio.h>
#include "../src/video_layout.h"

int main(void) {
    assert(TEXT_VRAM_GFX_ADDR == 0x3000);
    assert(TEXT_VRAM_MAP_ADDR == 0x6800);
    assert(TEXT_VRAM_OFFSET == 0x0100);
    assert(TEXT_VRAM_GFX_ADDR != TEXT_VRAM_MAP_ADDR);

    puts("test_video_layout: ok");
    return 0;
}
