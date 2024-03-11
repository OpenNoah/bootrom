#pragma once

void lcd_init(void);
void lcd_show_bitmap(const void *img);
void *lcd_get_fb();
void lcd_swap_fb();
