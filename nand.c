#include "config.h"
#include "nand.h"
#include "uart.h"
#include "gpio.h"
#include "io.h"

// Unit: 1ns
#define T_TO_CLK(t)	        DIV_CEIL((uint32_t)(t) * MEM_CLK_MHZ, 1000)

#define EMC_SMCR(n)         ((_IO uint32_t *)(SRAM_BASE + ((n) - 1) * 4))

#define NAND_BANK_BASE(n)   ((n) == 1 ? PA_TO_KSEG1(0x18000000) : \
                             (n) == 2 ? PA_TO_KSEG1(0x14000000) : \
                             (n) == 3 ? PA_TO_KSEG1(0x0c000000) : \
                                        PA_TO_KSEG1(0x08000000))
#if JZ4740
#define NAND_ADDR_PORT(n)       ((_IO uint8_t *)(NAND_BANK_BASE(n) + 0x00010000))   // A16
#define NAND_CMD_PORT(n)        ((_IO uint8_t *)(NAND_BANK_BASE(n) + 0x00008000))   // A15
#elif JZ4755
#define NAND_ADDR_PORT(n)       ((_IO uint8_t *)(NAND_BANK_BASE(n) + \
                                 (fw_args->is_busshare ? 0x00010000 : 0x10)))       // A16
#define NAND_CMD_PORT(n)        ((_IO uint8_t *)(NAND_BANK_BASE(n) + \
                                 (fw_args->is_busshare ? 0x00008000 : 0x08)))       // A15
#endif
#define NAND_DATA_PORT_8(n)     ((_IO uint8_t *)(NAND_BANK_BASE(n) + 0))
#define NAND_DATA_PORT_16(n)    ((_IO uint16_t *)(NAND_BANK_BASE(n) + 0))
#define NAND_DATA_PORT_32(n)    ((_IO uint32_t *)(NAND_BANK_BASE(n) + 0))

static struct nand_t {
    _IO uint32_t NFCSR;
#if JZ4740
    uint8_t _RESERVED0[0x100 - 0x54];
    _IO uint32_t NFECCR;
    _IO uint32_t NFECC;
    _IO uint32_t NFPAR[3];
    _IO uint32_t NFINTS;
    _IO uint32_t NFINTE;
    _IO uint32_t NFERR[4];
#endif
} * const nand = NAND_BASE;

static inline void nand_fce_assert(const unsigned bank)
{
    uint32_t mask = 0x03 << (2 * (bank - 1));
    nand->NFCSR = (nand->NFCSR & ~mask) | (0x03 << (2 * (bank - 1)));
}

static inline void nand_fce_restore(const unsigned bank)
{
    uint32_t mask = 0x03 << (2 * (bank - 1));
    nand->NFCSR = (nand->NFCSR & ~mask) | (0x01 << (2 * (bank - 1)));
}

void nand_init(void)
{
    const unsigned bank0 = config.nand.bank0;
    unsigned strv = T_TO_CLK(config.nand.tstrv);
    unsigned taw  = T_TO_CLK(config.nand.taw);
    unsigned tbp  = T_TO_CLK(config.nand.tbp);
    unsigned tah  = T_TO_CLK(config.nand.tah);
    unsigned tas  = T_TO_CLK(config.nand.tas);
    unsigned smcr = (strv << 24) | (taw << 20) | (tbp << 16) |
                    (tah << 12) | (tas << 8) | (config.nand.bw << 6);
    *EMC_SMCR(bank0) = smcr;
    nand_fce_restore(bank0);
    // Reset and wait
    gpio_nand_busy_catch();
    *NAND_CMD_PORT(bank0) = 0xff;
    gpio_nand_busy_wait();
}

void nand_print_id(void)
{
    const unsigned bank0 = config.nand.bank0;
    *NAND_CMD_PORT(bank0) = 0x90;
    *NAND_ADDR_PORT(bank0) = 0x00;

    uart_puts("NAND bank0 id: ");
    for (int i = 0; i < 6; i++) {
        uint8_t id = *NAND_DATA_PORT_8(bank0);
        uart_puthex(id, 2);
    }
    uart_puts("\r\n");
}

void nand_read_pages(void *dst, uint32_t start, uint32_t count, int oob)
{
    uart_puts(__func__);
    uart_puts(": 0x");
    uart_puthex(start, 8);
    uart_puts(" + 0x");
    uart_puthex(count, 8);
    uart_puts("\r\n");

    const unsigned bank = config.nand.bank0;
    uint32_t *buf32 = dst;
    while (count--) {
        gpio_nand_busy_catch();
        *NAND_CMD_PORT(bank) = 0x00;
        *NAND_ADDR_PORT(bank) = (0 >> 0) & 0xff;
        *NAND_ADDR_PORT(bank) = (0 >> 8) & 0xff;
        *NAND_ADDR_PORT(bank) = (start >> 0) & 0xff;
        *NAND_ADDR_PORT(bank) = (start >> 8) & 0xff;
        *NAND_ADDR_PORT(bank) = (start >> 16) & 0xff;
        *NAND_CMD_PORT(bank) = 0x30;
        gpio_nand_busy_wait();
        for (unsigned i = 0; i < config.nand.page / 4; i++)
            *buf32++ = *NAND_DATA_PORT_32(bank);
        if (oob) {
            for (unsigned i = 0; i < config.nand.oob / 4; i++)
                *buf32++ = *NAND_DATA_PORT_32(bank);
            if (config.nand.oob % 4) {
                // Unaligned OOB size
                uint8_t *p = (uint8_t *)buf32;
                for (unsigned i = 0; i < config.nand.oob % 4; i++)
                    p[i] = *NAND_DATA_PORT_8(bank);
                buf32++;
            }
        }
    }
}
