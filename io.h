#pragma once

#include <stdint.h>
#include "helper.h"
#include "config.h"

#define _I	volatile
#define _O	volatile
#define _IO	volatile

#define CGU_BASE	((void *)PA_TO_KSEG1(0x10000000))
#define LCD_BASE	((void *)PA_TO_KSEG1(0x13050000))
#define EMC_BASE	((void *)PA_TO_KSEG1(0x13010000))
#define SRAM_BASE	((void *)PA_TO_KSEG1(0x13010014))
#define NAND_BASE	((void *)PA_TO_KSEG1(0x13010050))
#define SDRAM_BASE	((void *)PA_TO_KSEG1(0x13010080))
#if JZ4755
#define BCH_BASE    ((void *)PA_TO_KSEG1(0x130d0000))
#endif

#define UART0_BASE	((void *)PA_TO_KSEG1(0x10030000))
#define UART1_BASE	((void *)PA_TO_KSEG1(0x10031000))
#define UART2_BASE	((void *)PA_TO_KSEG1(0x10032000))
#define UART3_BASE	((void *)PA_TO_KSEG1(0x10033000))

#define I2C_BASE	((void *)PA_TO_KSEG1(0x10042000))

// 16MB before end of SDRAM
#define SDRAM_LOAD_BASE	((void *)PA_TO_KSEG0(0x3000000))

static _IO uint32_t *BCR = EMC_BASE;
