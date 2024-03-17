#pragma once

#include "helper.h"

#ifndef JZ4740
#define JZ4740 0
#endif

#ifndef JZ4755
#define JZ4755 0
#endif

#pragma pack(push, 1)
typedef struct {
    /* CPU ID */
    unsigned int  cpu_id;

    /* PLL args */
    unsigned char ext_clk;
    unsigned char cpu_speed;
    unsigned char phm_div;
    unsigned char use_uart;
    unsigned int  baudrate;
} fw_args_t;

extern const fw_args_t *fw_args;

#define EXT_CLK_MHZ	    (fw_args->ext_clk)
#define EXT_CLK_RATE	MHZ(fw_args->ext_clk)
#define SYS_CLK_MHZ	    (EXT_CLK_MHZ * fw_args->cpu_speed)
#define SYS_CLK_RATE	MHZ(SYS_CLK_MHZ)
#define MEM_CLK_MHZ	    (SYS_CLK_MHZ / 3)   // Max. 133MHz
#define MEM_CLK_RATE    (SDRAM_CLK_MHZ)
#define LCD_CLK_RATE	(SYS_CLK_RATE / 3)	// Max. 150MHz
#define MMC_CLK_RATE	MHZ(24)			// Max. 25MHz
#define BAUDRATE	    (fw_args->baudrate)
