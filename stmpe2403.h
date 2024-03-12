#pragma once

#include <stdint.h>

void stmpe2403_init();
uint16_t stmpe2403_read_id();
uint32_t stmpe2403_gpio_in();
void stmpe2403_gpio_out(uint32_t v);
void stmpe2403_gpio_dir(uint32_t v);

const uint32_t *stmpe2403_read_state();
