#include "io.h"
#include "lcd.h"
#include "gpio.h"
#include "config.h"

#if JZ4740
#pragma pack(push, 1)
typedef struct hw_lcd_t {
    _IO uint32_t LCDCFG;
    _IO uint32_t LCDVSYNC;
    _IO uint32_t LCDHSYNC;
    _IO uint32_t LCDVAT;
    _IO uint32_t LCDDAH;
    _IO uint32_t LCDDAV;

    _IO uint32_t LCDPS;
    _IO uint32_t LCDCLS;
    _IO uint32_t LCDSPL;
    _IO uint32_t LCDREV;
    _IO uint32_t _RESERVED0[2];

    _IO uint32_t LCDCTRL;
    _IO uint32_t LCDSTATE;
    _I  uint32_t LCDIID;
    _IO uint32_t _RESERVED1[1];

    _IO uint32_t LCDDA0;
    _I  uint32_t LCDSA0;
    _I  uint32_t LCDFID0;
    _I  uint32_t LCDCMD0;

    _IO uint32_t LCDDA1;
    _I  uint32_t LCDSA1;
    _I  uint32_t LCDFID1;
    _I  uint32_t LCDCMD1;
} hw_lcd_t;
#elif JZ4755
#pragma pack(push, 1)
typedef struct hw_lcd_t {
    /* 0x0000 */ _IO uint32_t LCDCFG;
    /* 0x0004 */ _IO uint32_t LCDVSYNC;
    /* 0x0008 */ _IO uint32_t LCDHSYNC;
    /* 0x000c */ _IO uint32_t LCDVAT;
    /* 0x0010 */ _IO uint32_t LCDDAH;
    /* 0x0014 */ _IO uint32_t LCDDAV;
    /* 0x0018 */ _IO uint32_t LCDPS;
    /* 0x001c */ _IO uint32_t LCDCLS;
    /* 0x0020 */ _IO uint32_t LCDSPL;
    /* 0x0024 */ _IO uint32_t LCDREV;
                     uint32_t RESERVED_VAR[2];
    /* 0x0030 */ _IO uint32_t LCDCTRL;
    /* 0x0034 */ _IO uint32_t LCDSTATE;
    /* 0x0038 */ _IO uint32_t LCDIID;
                     uint32_t RESERVED_VAR[1];
    /* 0x0040 */ _IO uint32_t LCDDA0;
    /* 0x0044 */ _IO uint32_t LCDSA0;
    /* 0x0048 */ _IO uint32_t LCDFID0;
    /* 0x004c */ _IO uint32_t LCDCMD0;
    /* 0x0050 */ _IO uint32_t LCDDA1;
    /* 0x0054 */ _IO uint32_t LCDSA1;
    /* 0x0058 */ _IO uint32_t LCDFID1;
    /* 0x005c */ _IO uint32_t LCDCMD1;
    /* 0x0060 */ _IO uint32_t LCDOFFS0;
    /* 0x0064 */ _IO uint32_t LCDPW0;
    /* 0x0068 */ _IO uint32_t LCDCNUM0;
    /* 0x006c */ _IO uint32_t LCDDESSIZE0;
    /* 0x0070 */ _IO uint32_t LCDOFFS1;
    /* 0x0074 */ _IO uint32_t LCDPW1;
    /* 0x0078 */ _IO uint32_t LCDCNUM1;
    /* 0x007c */ _IO uint32_t LCDDESSIZE1;
                     uint32_t RESERVED_VAR[4];
    /* 0x0090 */ _IO uint32_t LCDRGBC;
                     uint32_t RESERVED_VAR[3];
    /* 0x0100 */ _IO uint16_t LCDOSDC;
                     uint16_t RESERVED_VAR[1];
    /* 0x0104 */ _IO uint16_t LCDOSDCTRL;
                     uint16_t RESERVED_VAR[1];
    /* 0x0108 */ _IO uint16_t LCDOSDS;
                     uint16_t RESERVED_VAR[1];
    /* 0x010c */ _IO uint32_t LCDBGC;
    /* 0x0110 */ _IO uint32_t LCDKEY0;
    /* 0x0114 */ _IO uint32_t LCDKEY1;
    /* 0x0118 */ _IO uint8_t  LCDALPHA;
                     uint8_t  RESERVED_VAR[3];
    /* 0x011c */ _IO uint32_t LCDIPUR;
    /* 0x0120 */ _IO uint32_t LCDXYP0;
    /* 0x0124 */ _IO uint32_t LCDXYP1;
    /* 0x0128 */ _IO uint32_t LCDSIZE0;
    /* 0x012c */ _IO uint32_t LCDSIZE1;
} hw_lcd_t;
#endif

static hw_lcd_t *lcd = LCD_BASE;

struct lcd_desc_t {
    uint32_t da;
    uint32_t sa;
    uint32_t fid;
    uint32_t cmd;
#if JZ4755
    uint32_t offs;
    uint32_t pw;
    uint32_t cnum;
    uint32_t dessize;
#endif
};

static _IO struct lcd_desc_t *desc[2];
static _IO uint32_t *buf[2];

static int desc_idx = 0;

void lcd_init(void)
{
    unsigned lcd_fmt = 0;
    if (config.lcd.bus_format == MEDIA_BUS_FMT_RGB565_1X16)
        lcd_fmt = 0;            // 16-bit TFT panel
    else if (config.lcd.bus_format == MEDIA_BUS_FMT_RGB666_1X18)
        lcd_fmt = BIT(7) | 0;   // 18-bit TFT panel
#if 0   // TODO Why 24-bit mode does not work???
    else if (config.lcd.bus_format == MEDIA_BUS_FMT_RGB888_1X24)
        lcd_fmt = BIT(6) | 0;   // 24-bit TFT panel
#else
    else if (config.lcd.bus_format == MEDIA_BUS_FMT_RGB888_1X24)
        lcd_fmt = BIT(7) | 0;   // Use it as 18-bit TFT panel for now
#endif
    else if (config.lcd.bus_format == MEDIA_BUS_FMT_RGB888_3X8)
        lcd_fmt = 0b1100;       // 8-bit serial TFT panel

    // Disable LCD controller
    lcd->LCDCTRL = BIT(4);
    while (lcd->LCDCTRL & BIT(3));
    // Configure LCD controller
    lcd->LCDCFG = (0 << 31) |
              //BIT(23) | BIT(22) | BIT(21) | BIT(20) |
              (0 << 19) | (0 << 18) | (0 << 17) |
              (0 << 16) | (0 << 15) | (0 << 14) | (0 << 13) |
              (0 << 12) |
#if JZ4755
              BIT(28) |     // 8-word new descriptor
#endif
              (!!(config.lcd.flags & DRM_MODE_FLAG_NHSYNC) << 11) |
              (!!(config.lcd.bus_flags & DRM_BUS_FLAG_PIXDATA_DRIVE_NEGEDGE) << 10) |
              (!!(config.lcd.bus_flags & DRM_BUS_FLAG_DE_LOW) << 9) |
              (!!(config.lcd.flags & DRM_MODE_FLAG_NVSYNC) << 8) | lcd_fmt;
    lcd->LCDVSYNC = config.lcd.vsync_end - config.lcd.vsync_start;
    lcd->LCDHSYNC = config.lcd.hsync_end - config.lcd.hsync_start;
    lcd->LCDVAT = (config.lcd.htotal << 16) | config.lcd.vtotal;
    lcd->LCDDAH = ((config.lcd.htotal - config.lcd.hsync_start) << 16) |
              (config.lcd.htotal - (config.lcd.hsync_start - config.lcd.hdisplay));
    lcd->LCDDAV = ((config.lcd.vtotal - config.lcd.vsync_start) << 16) |
              (config.lcd.vtotal - (config.lcd.vsync_start - config.lcd.vdisplay));
    lcd->LCDCTRL = (0b10 << 28) | (0 << 27) | (1 << 26) | 0b101;

    // LCD buffers
    static const unsigned align = 16 * 4;
    static const unsigned bpp = 4;
    uint32_t bufs = kseg0_to_kseg1(alloc(
        2 * (config.lcd.vdisplay * config.lcd.hdisplay * bpp +
             sizeof(struct lcd_desc_t)) + align));
    bufs += (align - (bufs % align)) % align;
    buf[0] = (void *)bufs;
    bufs += config.lcd.vdisplay * config.lcd.hdisplay * bpp;
    buf[1] = (void *)bufs;
    bufs += config.lcd.vdisplay * config.lcd.hdisplay * bpp;
    desc[0] = (void *)bufs;
    bufs += sizeof(struct lcd_desc_t);
    desc[1] = (void *)bufs;
    bufs += sizeof(struct lcd_desc_t);

    // LCD buffer descriptor
    desc[0]->da = kseg1_to_pa((void *)desc[0]);
    desc[0]->sa = kseg1_to_pa((void *)buf[0]);
    desc[0]->fid = 0;
    desc[0]->cmd = (config.lcd.vdisplay * config.lcd.hdisplay * bpp) / 4;
#if JZ4755
    desc[0]->offs = 0;
    desc[0]->pw = 0;
    desc[0]->cnum = 0;
    desc[0]->dessize = 0;
#endif
    lcd->LCDDA0 = kseg1_to_pa((void *)desc[0]);
    lcd->LCDDA1 = kseg1_to_pa((void *)desc[0]);

    // Test pattern
    for (unsigned y = 0; y < config.lcd.vdisplay; y++)
        for (unsigned x = 0; x < config.lcd.hdisplay; x++)
            buf[0][y * config.lcd.hdisplay + x] =
                ((~x & 0xff) << 16) |
                (( y & 0xff) <<  8) |
                (( x & 0xff) <<  0);

    for (unsigned y = 0; y < config.lcd.vdisplay; y++) {
        buf[0][y * config.lcd.hdisplay + 0] =
            (0x00 << 16) |
            (0x00 <<  8) |
            (0xff <<  0);
        buf[0][y * config.lcd.hdisplay + config.lcd.hdisplay - 1] =
            (0xff << 16) |
            (0xff <<  8) |
            (0x00 <<  0);
    }

    for (unsigned x = 0; x < config.lcd.hdisplay; x++) {
        buf[0][0 * config.lcd.hdisplay + x] =
            (0x00 << 16) |
            (0xff <<  8) |
            (0x00 <<  0);
        buf[0][(config.lcd.vdisplay - 1) * config.lcd.hdisplay + x] =
            (0xff << 16) |
            (0x00 <<  8) |
            (0xff <<  0);
    }

    // Enable LCD controller
    lcd->LCDSTATE = 0;
    lcd->LCDCTRL |= 1 << 3;

    gpio_lcd_enable(1);
}
