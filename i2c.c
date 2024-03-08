#include "config.h"
#include "i2c.h"
#include "cgu.h"
#include "io.h"

#define I2C_CLK_RATE    100000
#define I2C_TIMEOUT

#pragma packed(push, 1)
struct i2c_t {
    _IO uint8_t  I2CDR;
        uint8_t  RESERVED_VAR[3];
    _IO uint8_t  I2CCR;
        uint8_t  RESERVED_VAR[3];
    _IO uint8_t  I2CSR;
        uint8_t  RESERVED_VAR[3];
    _IO uint16_t I2CGR;
        uint8_t  RESERVED_VAR[2];
};

static struct i2c_t * const i2c = I2C_BASE;

void i2c_init()
{
    // Disable I2C
    i2c->I2CCR = 0;
    // Configure clock rate
    i2c->I2CGR = DIV_CEIL(cgu_ex_clk_rate(), 16 * I2C_CLK_RATE) - 1;
    // Enable I2C
    i2c->I2CCR = BIT(0);
}

int i2c_probe(uint8_t addr)
{
    // STA = 1, STO = 0, AC = 1, I2CE = 1
    i2c->I2CCR = BIT(3) | BIT(1) | BIT(0);
    // Address byte (write)
    i2c->I2CDR = (addr << 1) | 0;
    // DRF = 1
    i2c->I2CSR = BIT(1);
    // Wait until TEND == 1
    while (i2c->I2CSR & BIT(1));
    while (!(i2c->I2CSR & BIT(2)));
    // Read ACK
    int ack = i2c->I2CSR & BIT(0);
    // STA = 0, STO = 1, AC = 1, I2CE = 1
    i2c->I2CCR = BIT(2) | BIT(1) | BIT(0);
    return ack;
}
