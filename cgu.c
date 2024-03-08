#include "config.h"
#include "io.h"
#include "cgu.h"

#if JZ4740
#pragma packed(push, 1)
struct cgu_t {
    _IO uint32_t CPCCR;
    uint32_t _RESERVED0[3];
    _IO uint32_t CPPCR;
    uint32_t _RESERVED1[19];
    _IO uint32_t I2SCDR;
    _IO uint32_t LPCDR;
    _IO uint32_t MSCCDR;
    _IO uint32_t UHCCDR;
    uint32_t _RESERVED2[1];
    _IO uint32_t SSICDR;
};

#elif JZ4755
#pragma packed(push, 1)
struct cgu_t {
    /* 00 */ _IO uint32_t CPCCR;
    /* 04 */ _IO uint32_t LCR;
                  uint32_t RESERVED_VAR[2];
    /* 10 */ _IO uint32_t CPPCR;
    /* 14 */ _IO uint32_t CPPSR;
                 uint32_t RESERVED_VAR[2];
    /* 20 */ _IO uint32_t CLKGR;
    /* 24 */ _IO uint32_t OPCR;
                 uint32_t RESERVED_VAR[14];
    /* 60 */ _IO uint32_t I2SCDR;
    /* 64 */ _IO uint32_t LPCDR;
    /* 68 */ _IO uint32_t MSCCDR;
                 uint32_t RESERVED_VAR[2];
    /* 74 */ _IO uint32_t SSICDR;
                 uint32_t RESERVED_VAR[1];
    /* 7c */ _IO uint32_t CIMCDR;
};

#define CGU_CLKGR_UART0     BIT(0)
#define CGU_CLKGR_TCU       BIT(1)
#define CGU_CLKGR_RTC       BIT(2)
#define CGU_CLKGR_I2C       BIT(3)
#define CGU_CLKGR_SSI       BIT(4)
#define CGU_CLKGR_AIC       BIT(5)
#define CGU_CLKGR_MSC0      BIT(6)
#define CGU_CLKGR_SADC      BIT(7)
#define CGU_CLKGR_CIM       BIT(8)
#define CGU_CLKGR_LCD       BIT(9)
#define CGU_CLKGR_UDC       BIT(10)
#define CGU_CLKGR_BCH       BIT(11)
#define CGU_CLKGR_DMAC      BIT(12)
#define CGU_CLKGR_IPU       BIT(13)
#define CGU_CLKGR_UART1     BIT(14)
#define CGU_CLKGR_UART2     BIT(15)
#define CGU_CLKGR_MSC1      BIT(16)
#define CGU_CLKGR_TSSI      BIT(17)
#define CGU_CLKGR_TVE       BIT(18)
#define CGU_CLKGR_MC        BIT(19)
#define CGU_CLKGR_ME        BIT(20)
#define CGU_CLKGR_DB        BIT(21)
#define CGU_CLKGR_IDCT      BIT(22)
#define CGU_CLKGR_AHB1      BIT(23)
#define CGU_CLKGR_AUX_CPU   BIT(24)

#endif

static struct cgu_t * const cgu = CGU_BASE;

void cgu_pll_init(void)
{
#if JZ4740
#error TODO
#elif JZ4755
    // Switch everything to EXTCLK first to reconfigure PLL
    cgu->CPPCR = 0;

    // Configure PLL
    // PLL enabled but bypassed
    const unsigned long n = 2;
    const unsigned long m = fw_args->cpu_speed * n;
    cgu->CPPCR = ((m - 2) << 23) | ((n - 2) << 18) | (0 << 16) |
            (1 << 9) | (1 << 8) | (0x11 << 0);
    // System clock dividers
    const unsigned long udiv = 28;	// UDC freq should be 12MHz
    const unsigned long cdiv = 1;
    const unsigned long mdiv = 3; //fw_args->phm_div + 1;
    const unsigned long h0div = mdiv;
    const unsigned long h1div = mdiv;
    const unsigned long pdiv = mdiv;
    cgu->CPCCR = (1 << 30) | ((udiv - 1) << 23) | (1 << 22) | (1 << 21) |
            ((h1div - 1) << 16) | ((mdiv - 1) << 12) | ((pdiv - 1) << 8) |
            ((h0div - 1) << 4) | ((cdiv - 1) << 0);

    // Wait for PLL stable
    while (!(cgu->CPPCR & (1 << 10)));
    // Now, switch system clock to PLL
    cgu->CPPCR &= ~(1 << 9);

#if VARIANT == VARIANT_D88
    // Disable unused peripherals
    cgu->CLKGR |= CGU_CLKGR_MSC0 | CGU_CLKGR_TCU;
    // Enable all device peripherals
    cgu->CLKGR &= ~(CGU_CLKGR_MSC1 | CGU_CLKGR_UART1 | CGU_CLKGR_LCD | CGU_CLKGR_I2C);
#endif
#endif
}

uint32_t cgu_ex_clk_rate(void)
{
#if JZ4740
    return SYS_CLK_RATE;
#elif JZ4755
    unsigned div2 = cgu->CPCCR & (1 << 30) ? 2 : 1;
    return SYS_CLK_RATE / div2;
#endif
}
