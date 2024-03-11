#include <stdint.h>
#include "keyboard_test.h"
#include "lcd.h"
#include "helper.h"

#if STAGE == 2

static const uint8_t background[] = {
#include "image_keyboard.h"
};

struct {
    uint32_t y, x, h, w;
} key_regions[] = {
#include "image_keyboard_regions.h"
};

void keyboard_test()
{
    lcd_show_bitmap(background);

    // Screen (image) size
    uint32_t sw = key_regions[0].w;
    uint32_t sh = key_regions[0].h;

    // Copy background to new frame buffer
    uint32_t *fb = lcd_get_fb();
    const uint32_t *bg = (const uint32_t *)&background[0];
    for (uint32_t i = 0; i < sw * sh; i++)
        fb[i] = bg[i];

    // Draw keyboard regions
    for (uint32_t i = 1; i < ARRAY_SIZE(key_regions); i++) {
        for (uint32_t y = 0; y < key_regions[i].h; y++) {
            for (uint32_t x = 0; x < key_regions[i].w; x++) {
                uint32_t ofs = (key_regions[i].y + y) * sw + (key_regions[i].x + x);
                uint32_t r = ((i / 16) % 8) * 32;
                uint32_t g = ((i /  8) % 8) * 32;
                uint32_t b = ((i /  1) % 8) * 32;
                fb[ofs] = (r << 16) | (g << 8) | b;
                //fb[ofs] = ~fb[ofs];
            }
        }
    }

    // Done
    lcd_swap_fb();
}

#endif
