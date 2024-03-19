#include <stdint.h>
#include "io.h"
#include "config.h"
#include "gpio.h"

#define GPIOA_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010000))
#define GPIOB_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010100))
#define GPIOC_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010200))
#define GPIOD_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010300))
#define GPIOE_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010400))
#define GPIOF_BASE  ((gpio_t *)PA_TO_KSEG1(0x10010500))

#pragma pack(push, 1)
typedef struct gpio_t {
    struct gpio_port_t {
        _I uint32_t D;
        _O uint32_t S;
        _O uint32_t C;
           uint32_t _RESV;
    } PIN, DAT, IM, PE, FUN, SEL, DIR, TRG, FLG;
} gpio_t;

static gpio_t * const gpa = GPIOA_BASE;
static gpio_t * const gpb = GPIOB_BASE;
static gpio_t * const gpc = GPIOC_BASE;
static gpio_t * const gpd = GPIOD_BASE;
static gpio_t * const gpe = GPIOE_BASE;
static gpio_t * const gpf = GPIOF_BASE;

#if VARIANT == VARIANT_D88

//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOA_FUN   0b11111111111111111111111111111111      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOA_SEL   0b00000000000000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOA_DIR   0b00000000000000000000000000000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOA_DAT   0b00000000000000000000000000000000      // Output data
#define GPIOA_PE    0b11111111111111111111111111111111      // Pull-up/down

//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOB_FUN   0b11111111111111111111111111111111      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOB_SEL   0b11111100000000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOB_DIR   0b00000000000000000000000000000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOB_DAT   0b00000000000000000000000000000000      // Output data
#define GPIOB_PE    0b11111111111111111111111111111111      // Pull-up/down

//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOC_FUN   0b00110000001000010000000000000000      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOC_SEL   0b00001000000000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOC_DIR   0b00000000100000000000000000000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOC_DAT   0b00000000100000000000000000000000      // Output data
#define GPIOC_PE    0b00110000101000010000000000000000      // Pull-up/down
//      Check         -x---xx?-x-xxx?-xxxxxxxxxxxxxxxx
// PC23: LCD DISP
// PC24: LCD backlight control? likely not

#if 1
//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOD_FUN   0b00001111111111111111111111111111      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOD_SEL   0b00000011110000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOD_DIR   0b00000000000000000000000000000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOD_DAT   0b00000000000000000000000000000000      // Output data
#define GPIOD_PE    0b00001111111111111111111111111111      // Pull-up/down
//      Check         xxxx----------------------------
#else
// RGB565 mode for comparison
//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOD_FUN   0b00000000001111111110111111111110      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOD_SEL   0b00000000000000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOD_DIR   0b00001111110000000001000000000001      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOD_DAT   0b00000000000000000000000000000000      // Output data
#define GPIOD_PE    0b00001111111111111111111111111111      // Pull-up/down
//      Check         xxxx----------------------------
#endif

//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOE_FUN   0b00000010100000000011000000000000      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOE_SEL   0b00000010100000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOE_DIR   0b00000000000000000000000110000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOE_DAT   0b00000000000000000000000110000000      // Output data
#define GPIOE_PE    0b01000010000000000000000110000000      // Pull-up/down
//      Check         ?-xxxx-x-?xx??xxxx--???--??-???-
// PE0: Lid detect
// PE4: MMC card detect
// PE7: LCD PWR EN
// PE8: Keyboard EN
// PE12: I2C SDA
// PE13: I2C SCK

//      GPIO           3         2         1         0
//      GPIO          10987654321098765432109876543210
#define GPIOF_FUN   0b00000000000000000000000000000000      // Function  - 0: GPIO/INT,    1: AF0/AF1
#define GPIOF_SEL   0b00000000000000000000000000000000      // Select    - 0: GPIO/AF0,    1: INT/AF1
#define GPIOF_DIR   0b00000000000000000001000000000000      // Direction - 0: IN/LOW/FALL, 1: OUT/HIGH/RISE
#define GPIOF_DAT   0b00000000000000000000000000000000      // Output data
#define GPIOF_PE    0b00000000000000000001000000000000      // Pull-up/down
//      Check         xxxxxxxxxxxxxxxxxxx-??xxxxxxxxxx
// PF12: LCD backlight

// NAND busy: Input, level trigger
#define GPIOC_PINS_NAND_BUSY    27

#endif

#define GPIO_CONFIG(port)                       \
    do {                                        \
        gpio_t * const gp = GPIO##port##_BASE;  \
        gp->IM.S  = 0xffffffff;                 \
        gp->DAT.C = ~GPIO##port##_DAT;          \
        gp->DAT.S =  GPIO##port##_DAT;          \
        gp->PE.C  = ~GPIO##port##_PE;           \
        gp->PE.S  =  GPIO##port##_PE;           \
        gp->DIR.C = ~GPIO##port##_DIR;          \
        gp->DIR.S =  GPIO##port##_DIR;          \
        gp->FUN.C = ~GPIO##port##_FUN;          \
        gp->FUN.S =  GPIO##port##_FUN;          \
        gp->SEL.C = ~GPIO##port##_SEL;          \
        gp->SEL.S =  GPIO##port##_SEL;          \
    } while (0)

void gpio_init(void)
{
    GPIO_CONFIG(A);
    GPIO_CONFIG(B);
    GPIO_CONFIG(C);
    GPIO_CONFIG(D);
    GPIO_CONFIG(E);
    GPIO_CONFIG(F);

    // Level trigger for NAND busy pin
    gpc->TRG.C = BIT(GPIOC_PINS_NAND_BUSY);
}

int gpio_nand_rb(void)
{
    return gpc->PIN.D & BIT(GPIOC_PINS_NAND_BUSY);
}

void gpio_nand_busy_catch(void)
{
    // Low level trigger
    gpc->DIR.C = BIT(GPIOC_PINS_NAND_BUSY);
    // FLG.C is actually located at DAT.S
    gpc->DAT.S = BIT(GPIOC_PINS_NAND_BUSY);
}

void gpio_nand_busy_wait(void)
{
    // Wait for low level
    while (!(gpc->FLG.D & BIT(GPIOC_PINS_NAND_BUSY)));
    // High level trigger
    gpc->DIR.S = BIT(GPIOC_PINS_NAND_BUSY);
    // FLG.C is actually located at DAT.S
    gpc->DAT.S = BIT(GPIOC_PINS_NAND_BUSY);
    // Wait for high level
    while (!(gpc->FLG.D & BIT(GPIOC_PINS_NAND_BUSY)));
}
