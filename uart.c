#include "config.h"
#include "io.h"
#include "gpio.h"
#include "cgu.h"
#include "uart.h"

#define LINE_BUFFER_SIZE	32

struct hw_uart_t {
	union {
		struct {
			union {
				_I uint32_t URBR;
				_O uint32_t UTHR;
			};
			_IO uint32_t UIER;
		};
		struct {
			_IO uint32_t UDLLR;
			_IO uint32_t UDLHR;
		};
	};
	union {
		_I uint32_t UIIR;
		_O uint32_t UFCR;
	};
	_IO uint32_t ULCR;
	_IO uint32_t UMCR;
	_I uint32_t ULSR;
	_I uint32_t UMSR;
	_IO uint32_t USPR;
	_IO uint32_t ISR;
	_IO uint32_t UMR;
	_IO uint32_t UACR;
};

#if JZ4740
static struct hw_uart_t * const uart = UART0_BASE;
#elif JZ4755
static struct hw_uart_t * const uart = UART1_BASE;
#endif

void uart_init(void)
{
	// Disable UART
	uart->UFCR = 0;
	// DLAB = 1, 8-bit
	uart->ULCR = 0b10000011;

#if 0
	// Baud rate calculations
	if (cgu_ex_clk_rate() == MHZ(12) && BAUDRATE == 115200) {
		uart->UMR = 17;
		uart->UACR = 594;
		uart->UDLLR = 6;
		uart->UDLHR = 0;
	} else {
		// Anyway, I haven't implemented baudrate calculations
		uart->UMR = 17;
		uart->UACR = 594;
		uart->UDLLR = 6;
		uart->UDLHR = 0;
	}
#endif

	// DLAB = 0, 8-bit
	uart->ULCR = 0b00000011;
	// Enable UART, clear FIFO
	uart->UFCR = 0b00010111;
}

void uart_putc(char c)
{
	while (!(uart->ULSR & (1 << 6)));	// TEMP
	uart->UTHR = c;
}

void uart_puts(const char *s)
{
	while (*s != '\0')
		uart_putc(*s++);
}

void uart_puthex(uint32_t v, int w)
{
	for (int i = 0; i < w; i++) {
		uint8_t fv = (v >> (4 * (w - 1 - i))) & 0xf;
		char c = fv < 10 ? fv + '0' : fv + 'a' - 10;
		uart_putc(c);
	}
}

void uart_putdec(uint32_t v)
{
	uint32_t base = 1000000000;
	while (v && v / base == 0)
		base /= 10;
	while (base) {
		uint32_t vv = v / base;
		v -= vv * base;
		base /= 10;
		char c = vv + '0';
		uart_putc(c);
	}
}

char uart_getc(void)
{
	while (!(uart->ULSR & 1));
	return uart->URBR;
}

char *uart_get_line(void)
{
	static char line[LINE_BUFFER_SIZE];
	int i = 0;
	for (;;) {
		char c = uart_getc();
		if (c == '\x7f') {
			if (i != 0) {
				uart_putc(c);
				i--;
			}
		} else if (c == '\r' || c == '\n') {
			uart_puts("\r\n");
			break;
		} else {
			if (i != LINE_BUFFER_SIZE - 1) {
				uart_putc(c);
				line[i++] = c;
			}
		}
	}
	line[i] = 0;
	return line;
}
