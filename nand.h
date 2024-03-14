#pragma once

#include <stdint.h>

void nand_init(void);
void nand_print_id(void);
void nand_read_pages(void *dst, uint32_t start, uint32_t count, int oob);
