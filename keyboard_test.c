#include <stdint.h>
#include "keyboard_test.h"
#include "lcd.h"

static const uint8_t background[] = {
#include "image_keyboard.h"
};

void keyboard_test()
{
    lcd_show_bitmap(background);
}
