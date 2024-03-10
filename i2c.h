#pragma once

#include <stdint.h>

void i2c_init();

int i2c_probe(uint8_t addr);
void i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint32_t len);
void i2c_write(uint8_t addr, uint8_t reg, const uint8_t *buf, uint32_t len);
