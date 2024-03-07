#pragma once

#define CGU_CLKGR_MSC0      (1 << 6)
#define CGU_CLKGR_LCD       (1 << 9)
#define CGU_CLKGR_UART1     (1 << 14)

void cgu_pll_init(void);
void cgu_clk_enable(uint32_t clkgr);
uint32_t cgu_uart_clk_rate(void);
