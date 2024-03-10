#include "stmpe2403.h"
#include "config.h"
#include "i2c.h"

#define I2C_ADDR            0x42

#define REG_SC_SYSCON       0x02
#define REG_SC_SYSCON2      0x03
#define REG_SC_CHIP_ID      0x80
#define REG_SC_VERSION_ID   0x81

#define REG_GPIO_GPSR       0x83
#define REG_GPIO_GPDR       0x89
#define REG_GPIO_GPMR       0xa2

//      GPIO             2         1         0
//      GPIO          321098765432109876543210
#define GPIO_ROW    0b000000010111000000010000
#define GPIO_COL    0b000111100000111110001111

#pragma pack(push, 1)
typedef struct {
    /* 0x83 */ uint8_t GPSR[3];
    /* 0x86 */ uint8_t GPCR[3];
    /* 0x89 */ uint8_t GPDR[3];
    /* 0x8c */ uint8_t GPEDR[3];
    /* 0x8f */ uint8_t GPRER[3];
    /* 0x92 */ uint8_t GPFER[3];
    /* 0x95 */ uint8_t GPPUR[3];
    /* 0x98 */ uint8_t GPPDR[3];
    /* 0x9b */ uint8_t GPAFR_U[3];
    /* 0x9e */ uint8_t GPAFR_L[3];
    /* 0xa1 */ uint8_t MUX_CTRL;
    /* 0xa2 */ uint8_t GPMR[3];
    /* 0xa5 */ uint8_t COMPAT2401;
} stmpe2403_gpio_t;

void stmpe2403_init()
{
    // Enable GPIO and KPC
    uint8_t syscon = 0x0a;
    i2c_write(I2C_ADDR, REG_SC_SYSCON, &syscon, 1);

    // Initialise GPIOs
    stmpe2403_gpio_t gpio = {0};
    // Set output values
    gpio.GPSR[0]  = (GPIO_COL >> 16) & 0xff;
    gpio.GPSR[1]  = (GPIO_COL >>  8) & 0xff;
    gpio.GPSR[2]  = (GPIO_COL >>  0) & 0xff;
    // Set directions
    gpio.GPDR[0]  = (GPIO_COL >> 16) & 0xff;
    gpio.GPDR[1]  = (GPIO_COL >>  8) & 0xff;
    gpio.GPDR[2]  = (GPIO_COL >>  0) & 0xff;
    // Set pull-downs
    gpio.GPPDR[0] = (GPIO_ROW >> 16) & 0xff;
    gpio.GPPDR[1] = (GPIO_ROW >>  8) & 0xff;
    gpio.GPPDR[2] = (GPIO_ROW >>  0) & 0xff;
    i2c_write(I2C_ADDR, REG_GPIO_GPSR, (const uint8_t *)&gpio, 0xa6 - 0x83);
}

uint16_t stmpe2403_read_id()
{
    uint8_t id[2];
    i2c_read(I2C_ADDR, REG_SC_CHIP_ID, id, 2);
    return (id[0] << 8) | id[1];
}

uint32_t stmpe2403_gpio_in()
{
    uint8_t buf[3];
    i2c_read(I2C_ADDR, REG_GPIO_GPMR, buf, 3);
    return (buf[0] << 16) | (buf[1] << 8) | (buf[2] << 0);
}

void stmpe2403_gpio_out(uint32_t v)
{
    uint8_t buf[6];
    buf[0] = v >> 16;
    buf[1] = v >>  8;
    buf[2] = v >>  0;
    buf[3] = ~buf[0];
    buf[4] = ~buf[1];
    buf[5] = ~buf[2];
    i2c_write(I2C_ADDR, REG_GPIO_GPSR, buf, 6);
}

void stmpe2403_gpio_dir(uint32_t v)
{
    uint8_t buf[3];
    buf[0] = v >> 16;
    buf[1] = v >>  8;
    buf[2] = v >>  0;
    i2c_write(I2C_ADDR, REG_GPIO_GPDR, buf, 3);
}
