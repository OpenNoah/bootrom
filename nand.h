#pragma once

#include <stdint.h>

void nand_init();
void nand_print_id();
void nand_boot();
void nand_read_pages(void *dst, uint32_t start, uint32_t count, int oob);
