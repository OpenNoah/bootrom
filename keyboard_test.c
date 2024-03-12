#include <stdint.h>
#include "keyboard_test.h"
#include "stmpe2403.h"
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

struct {
    uint32_t row, col;
} key_matrix[] = {
    {0, 0},     // Backlight
    {0, 1},     // Power
    // Row
    {13, 9}, {13, 10}, {13, 0}, {13, 1}, {13, 8}, {13, 11},
    {13, 2}, {13, 3}, {13, 17}, {13, 19}, {13, 7}, {16, 7},
    // Row
    {16, 9}, {16, 10}, {16, 0}, {16, 1}, {16, 8}, {16, 11},
    {16, 2}, {16, 18}, {16, 3}, {16, 17}, {16, 19},
    // Row
    {12, 9}, {14, 9}, {14, 10}, {14, 0}, {14, 1}, {14, 8},
    {14, 11}, {14, 2}, {14, 18}, {14, 3}, {14, 7},
    // Row
    {14, 20}, {12, 10}, {12, 0}, {12, 1}, {12, 8}, {12, 11},
    {12, 2}, {12, 18}, {4, 11}, {12, 19}, {12, 20},
    // Row
    {4, 9}, {4, 10}, {4, 1}, {4, 8}, {12, 7}, {4, 2},
    {4, 18}, {4, 3}, {4, 17}, {4, 19}, {4, 7},
};

void keyboard_test()
{
    lcd_show_bitmap(background);

    // Screen (image) size
    uint32_t sw = key_regions[0].w;
    uint32_t sh = key_regions[0].h;

    for (;;) {
        // Scan matrix state
        const uint32_t *scan = stmpe2403_read_state();

        // Copy background to new frame buffer
        uint32_t *fb = lcd_get_fb();
        const uint32_t *bg = (const uint32_t *)&background[0];
        for (uint32_t i = 0; i < sw * sh; i++)
            fb[i] = bg[i];

        // Scan result at top-left corner
        const uint32_t scale = 2;
        for (uint32_t y = 0; y < 24 * scale; y++) {
            for (uint32_t x = 0; x < 24 * scale; x++) {
                uint32_t ofs = y * sw + x;
                uint32_t row = y / scale;
                uint32_t col = x / scale;
                uint32_t clr = (scan[row] & BIT(col)) ? 0xb5e61d : 0x880015;
                fb[ofs] = clr;
            }
        }

        // Draw keyboard regions
        for (uint32_t i = 1; i < ARRAY_SIZE(key_regions); i++) {
            uint32_t row = key_matrix[i - 1].row;
            uint32_t col = key_matrix[i - 1].col;
            if (!(scan[row] & BIT(col)))
                continue;
            // Key pressed
            for (uint32_t y = 0; y < key_regions[i].h; y++) {
                for (uint32_t x = 0; x < key_regions[i].w; x++) {
                    uint32_t ofs = (key_regions[i].y + y) * sw + (key_regions[i].x + x);
                    fb[ofs] = ~fb[ofs];
                }
            }
        }

        // Done
        lcd_swap_fb();
    }
}

#endif
