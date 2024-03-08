#pragma once

#include "helper.h"

#include "drm_mode.h"
#include "drm_connector.h"
#include "media-bus-format.h"

// Variants
#define VARIANT_NP1500  0x1500
#define VARIANT_NP1501  0x1501
#define VARIANT_NP2150  0x2150
#define VARIANT_D88	    0x0088

#if VARIANT == VARIANT_NP1500
#define JZ4740	1
#define SDRAM_HY57V561620FTP_H
#define NAND_HY27UU08AG5M
#define LCD_PT035TN01
#elif VARIANT == VARIANT_NP1501
#define JZ4740  1
#define SDRAM_HY57V561620FTP_H
#define NAND_K9GAG08U0M
#define LCD_PT035TN01
#elif VARIANT == VARIANT_NP2150
#define JZ4740	1
#define SDRAM_HY57V561620FTP_H
#define NAND_K9GAG08U0M
#define LCD_AT043TN24
#elif VARIANT == VARIANT_D88
#define JZ4755	1
#define SDRAM_A3V56S40ETP_G6
#define NAND_K9GAG08U0M
#define LCD_LMS430HF15
#else
#error Unknown board variant
#endif

#ifndef JZ4740
#define JZ4740 0
#endif

#ifndef JZ4755
#define JZ4755 0
#endif

#pragma packed(push, 1)
typedef struct {
	/* CPU ID */
	unsigned int  cpu_id;

	/* PLL args */
	unsigned char ext_clk;
	unsigned char cpu_speed;
	unsigned char phm_div;
	unsigned char use_uart;
	unsigned int  baudrate;

	/* SDRAM args */
	unsigned char bus_width;
	unsigned char bank_num;
	unsigned char row_addr;
	unsigned char col_addr;
	unsigned char is_mobile;
	unsigned char is_busshare;

	/* debug args */
	unsigned char debug_ops;
	unsigned char pin_num;
	unsigned int  start;
	unsigned int  size;
} fw_args_t;

extern char _header;
static const fw_args_t *fw_args = (fw_args_t *)(&_header + 8);

#define EXT_CLK_MHZ	    (fw_args->ext_clk)
#define EXT_CLK_RATE	MHZ(fw_args->ext_clk)
#define SYS_CLK_MHZ	    (EXT_CLK_MHZ * fw_args->cpu_speed)
#define SYS_CLK_RATE	MHZ(SYS_CLK_MHZ)
#define SDRAM_CLK_MHZ	(SYS_CLK_MHZ / 3)   // Max. 133MHz
#define SDRAM_CLK_RATE	MHZ(SDRAM_CLK_MHZ)
#define LCD_CLK_RATE	(SYS_CLK_RATE / 3)	// Max. 150MHz
#define MMC_CLK_RATE	MHZ(24)			// Max. 25MHz
#define BAUDRATE	    (fw_args->baudrate)

struct sdram_config_t {
    // Parameters
    unsigned bw16;	// 0: 32-bit, 1: 16-bit
    unsigned bank4;	// 0: 2 banks per chip, 1: 4 banks per chip
    unsigned nrow;
    unsigned ncol;
    unsigned cas;
    unsigned burst;	// 0: burst read & write, 1: burst read only
    unsigned btype;	// 0: sequential, 1: interleave
    unsigned blen;	// burst length

    // Timing, unit: 0.1ns
    unsigned tref;	// Refresh cycle time
    unsigned trc;	// RAS cycle time
    unsigned trcd;	// RAS to CAS delay
    unsigned tras;	// RAS active time
    unsigned trp;	// RAS precharge time

    // Timing, unit: clocks
    unsigned dpl;	// Data-in to precharge
};

struct nand_config_t {
    unsigned bank;	// Chip select #
    unsigned block;	// Block size
    unsigned page;	// Page size
    unsigned oob;	// OOB bytes per page
};

struct lcd_config_t {
    unsigned clock;
    unsigned hdisplay;
    unsigned hsync_start;
    unsigned hsync_end;
    unsigned htotal;
    unsigned vdisplay;
    unsigned vsync_start;
    unsigned vsync_end;
    unsigned vtotal;
    unsigned flags;
    unsigned bus_format;
    unsigned bus_flags;
};

struct config_t {
    struct sdram_config_t sdram;
    struct nand_config_t nand;
    struct lcd_config_t lcd;
};

static const struct config_t config = {
    .sdram = {
#ifdef SDRAM_HY57V561620FTP_H
        .bw16 = 0,
        .bank4 = 1,
        .nrow = 13,
        .ncol = 9,
        .cas = 3,
        .burst = 0,
        .btype = 0,
        .blen = 0b010,	// burst length 4

        .tref = 64 * 1000 * 1000 * 10 / 8192,
        .trc = 630,
        .trcd = 200,
        .tras = 420,
        .trp = 200,
        .dpl = 2,
#elif defined(SDRAM_A3V56S40ETP_G6)
        .bw16 = 0,
        .bank4 = 1,
        .nrow = 13,
        .ncol = 9,
        .cas = 3,
        .burst = 0,
        .btype = 0,
        .blen = 0b010,	// burst length 4

        .tref = 64 * 1000 * 1000 * 10 / 8192,
        .trc = 600,
        .trcd = 180,
        .tras = 420,
        .trp = 180,
        .dpl = 2,
#endif
    },
    .nand = {
#ifdef NAND_K9GAG08U0M
        .bank = 1,
        .block = 512 * 1024,
        .page = 4 * 1024,
        .oob = 16 * 2 * 4,
#elif defined(NAND_HY27UU08AG5M)
        .bank = 1,
        .block = 256 * 1024,
        .page = 2 * 1024,
        .oob = 64,
#endif
    },
    .lcd = {
#ifdef LCD_PT035TN01
        .clock = 6035,
        .hdisplay = 320,
        .hsync_start = 320 + 10,
        .hsync_end = 320 + 10 + 1,
        .htotal = 320 + 10 + 1 + 50,
        .vdisplay = 240,
        .vsync_start = 240 + 10,
        .vsync_end = 240 + 10 + 1,
        .vtotal = 240 + 10 + 1 + 14,
        .flags = DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
        .bus_format = MEDIA_BUS_FMT_RGB888_3X8,
        .bus_flags = DRM_BUS_FLAG_DE_HIGH | DRM_BUS_FLAG_PIXDATA_DRIVE_NEGEDGE,
#elif defined(LCD_AT043TN24)
        .clock = 9000,
        .hdisplay = 480,
        .hsync_start = 480 + 2,
        .hsync_end = 480 + 2 + 41,
        .htotal = 480 + 2 + 41 + 2,
        .vdisplay = 272,
        .vsync_start = 272 + 2,
        .vsync_end = 272 + 2 + 10,
        .vtotal = 272 + 2 + 10 + 2,
        .flags = DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
        .bus_format = MEDIA_BUS_FMT_RGB666_1X18,
        .bus_flags = DRM_BUS_FLAG_DE_HIGH,
#elif defined(LCD_LMS430HF15)
        .clock = 9200,
        .hdisplay = 480,
        .hsync_start = 480 + 8,
        .hsync_end = 480 + 8 + 41,
        .htotal = 480 + 8 + 41 + 45,
        .vdisplay = 272,
        .vsync_start = 272 + 2,
        .vsync_end = 272 + 2 + 1,
        .vtotal = 272 + 2 + 1 + 11,
        .flags = DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
        .bus_format = MEDIA_BUS_FMT_RGB888_1X24,
        .bus_flags = DRM_BUS_FLAG_DE_HIGH,
#endif
    },
};

static const void *sdram_base = (void *)PA_TO_KSEG0(0);
static const uint32_t sdram_size = (4 / (1 + config.sdram.bw16)) *
    (1 << (config.sdram.nrow + config.sdram.ncol)) *
    (2 * (1 + config.sdram.bank4));
