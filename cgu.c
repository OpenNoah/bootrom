#include "config.h"
#include "io.h"
#include "cgu.h"

#define DIV_CEIL(a, b)	(((a) + (b) - 1) / (b))

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

	// Disable unused peripherals
	cgu->CLKGR |= CGU_CLKGR_MSC0;
#endif
}

void cgu_clk_enable(uint32_t clkgr)
{
	cgu->CLKGR &= ~clkgr;
}

uint32_t cgu_uart_clk_rate(void)
{
#if JZ4740
	return SYS_CLK_RATE;
#elif JZ4755
	unsigned div2 = cgu->CPCCR & (1 << 30) ? 2 : 1;
	return SYS_CLK_RATE / div2;
#endif
}
