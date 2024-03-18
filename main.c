#include <stdint.h>
#include "config.h"
#include "cgu.h"
#include "gpio.h"
#include "uart.h"
#include "nand.h"
#include "cp0.h"

static fw_args_t _fw_args = {
	.cpu_id = 0x4755,
	.ext_clk = 24,
	.cpu_speed = 14,
	.phm_div = 3,
	.use_uart = 1,
	.baudrate = 115200,
};

const fw_args_t *fw_args = &_fw_args;

void print_arch()
{
	uart_puts("Processor ID: 0x");
	uart_puthex(cp0_prid(), 8);
	uart_puts("\r\n");

	uint32_t configs[6];
	cp0_configs(configs);

	for (int i = 0; i < 6; i++) {
		uart_puts("CP0 Config ");
		uart_puthex(i, 1);
		uart_puts(": 0x");
		uart_puthex(configs[i], 8);
		uart_puts("\r\n");
	}
}

int main()
{
	cgu_pll_init();
	gpio_init();

	uart_init();
	uart_puts("\r\n*** usbboot stage1 JZ");
	uart_puthex(fw_args->cpu_id, 4);
	uart_puts(" ***\r\n");
	print_arch();

	nand_init();
	nand_print_id();
	for (;;);
}
