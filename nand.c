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

#if JZ4755
static struct bch_t {
    _I  uint32_t BHCR;
    _O  uint32_t BHCSR;
    _O  uint32_t BHCCR;
    _IO uint32_t BHCNT;
    _IO uint8_t  BHDR;
        uint8_t  RESERVED_VAR[3];
    _IO uint32_t BHPAR[4];
    _I  uint32_t BHINT;
    _I  uint32_t BHERR[4];
    _IO uint32_t BHINTE;
    _O  uint32_t BHINTES;
    _O  uint32_t BHINTEC;
} * const bch = BCH_BASE;
#endif

static struct {
    uint32_t correctable;
    uint32_t uncorrectable;
} ecc_count = {0};

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
        *NAND_ADDR_PORT(bank) = (start >>  0) & 0xff;
        *NAND_ADDR_PORT(bank) = (start >>  8) & 0xff;
        *NAND_ADDR_PORT(bank) = (start >> 16) & 0xff;
        *NAND_CMD_PORT(bank) = 0x30;
        gpio_nand_busy_wait();

        uint32_t page_data[config.nand.page / 4];
        for (unsigned i = 0; i < config.nand.page / 4; i++)
            page_data[i] = *NAND_DATA_PORT_32(bank);

        uint32_t oob_data[(config.nand.oob + 3) / 4];
        for (unsigned i = 0; i < config.nand.oob / 4; i++)
            oob_data[i] = *NAND_DATA_PORT_32(bank);
        if (config.nand.oob % 4) {
            // Unaligned OOB size
            oob_data[config.nand.oob / 4] = 0xffffffff;
            uint8_t *p = (uint8_t *)&oob_data[config.nand.oob / 4];
            for (unsigned i = 0; i < config.nand.oob % 4; i++)
                p[i] = *NAND_DATA_PORT_8(bank);
        }

        // D88 uses BCH hardware ECC
        uint32_t ecc_ofs = start <= 3 ? 3 : 24;
        for (unsigned block = 0; block < config.nand.page / 512; block++) {
            if (start >= 0x0800) {
                // If page data is all 0xff, then skip ECC correction
                // mtdblock may add oob data without ECC
                uint32_t data_mask = 0xffffffff;
                for (unsigned i = 0; i < 512 / 4; i++)
                    data_mask &= page_data[i];
                if (data_mask == 0xffffffff)
                    continue;
            }

            // BCH ECC decoding sequence
            // 1 Set BHCR.BCHE to 1 to enable BCH controller.
            // 2 Select 4-bit or 8-bit correction by setting BHCR.BSEL.
            // 3 Clear BHCR.ENCE to 0 to enable decoding.
            bch->BHCCR = BIT(3);
            // 4 Set BHCR.BRST to 1 to reset BCH controller.
            bch->BHCSR = BIT(2) | BIT(1) | BIT(0);
            // 5 Set BHCNT.DEC_COUNT to data block size in bytes.
            bch->BHCNT = (512 + 13 + (start >= 0x0800 ? 3 : 0)) << 16;
            // Clear error flags
            bch->BHINT = 0xff;
            // 6 Byte-write all data block to BHDR.
            for (unsigned i = 0; i < 512; i++)
                bch->BHDR = ((uint8_t *)&page_data[0])[block * 512 + i];
            // For rootfs etc., include oob area data
            if (start >= 0x0800)
                for (unsigned i = 0; i < 3; i++)
                    bch->BHDR = ((uint8_t *)&oob_data[0])[block * 3 + i];
            for (unsigned i = 0; i < 13; i++)
                bch->BHDR = ((uint8_t *)&oob_data[0])[ecc_ofs + block * 13 + i];
            // 7 Check BHINTS.DECF bit or by enabling decoding finish interrupt.
            while (!(bch->BHINT & BIT(3)));
            // 8 When decoding finishes, read out the status in BHINT and error report in BHERRn.
            uint32_t status = bch->BHINT;
            if (status & BIT(0)) {
                // Error occured
#if 0
                uart_puts("BCH ECC status: 0x");
                uart_puthex(bch->BHINT, 8);
                uart_puts(": 0x");
                uart_puthex(bch->BHERR[0], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHERR[1], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHERR[2], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHERR[3], 8);
                uart_puts("\r\n");
#endif
                uart_puts("BCH ECC error @ page 0x");
                uart_puthex(start, 8);
                uart_puts(" block ");
                uart_puthex(block, 1);
                uart_puts(": ");
                if (status & BIT(1)) {
                    // Uncorrectable error occured
                    uart_puts("Uncorrectable\r\n");
                    ecc_count.uncorrectable += 1;
                } else {
                    uint32_t err_bits = status >> 28;
                    uart_puthex(err_bits, 1);
                    uart_puts(" errors corrected\r\n");
                    ecc_count.correctable += err_bits;
                    while (err_bits--) {
                        uint16_t bit = (bch->BHERR[err_bits / 2] >> (16 * (err_bits % 2))) - 1;
                        if (bit < 512 * 8) {
                            // Error is in page data
                            bit += block * 512 * 8;
                            ((uint8_t *)&page_data[0])[bit / 8] ^= 1 << (bit % 8);
                        } else if (start >= 0x0800 && bit < (512 + 3) * 8) {
                            // Error is in oob data
                            bit -= 512 * 8;
                            bit += block * 3 * 8;
                            ((uint8_t *)&oob_data[0])[bit / 8] ^= 1 << (bit % 8);
                        } else {
                            // Error is in parity data
                            bit -= (512 + (start >= 0x0800 ? 3 : 0)) * 8;
                            bit += (ecc_ofs + block * 13) * 8;
                            ((uint8_t *)&oob_data[0])[bit / 8] ^= 1 << (bit % 8);
                        }
                    }
                }

#if 0
                // BCH ECC encoding sequence
                // 1 Set BHCR.BCHE to 1 to enable BCH controller.
                // 2 Select 4-bit or 8-bit correction by setting BHCR.BSEL.
                // 3 Set BHCR.ENCE to 1 to enable encoding.
                // 4 Set BHCR.BRST to 1 to reset BCH controller.
                bch->BHCSR = BIT(3) | BIT(2) | BIT(1) | BIT(0);
                // 5 Set BHCNT.ENC_COUNT to data block size in bytes.
                bch->BHCNT = 512 + (start >= 0x0800 ? 3 : 0);
                // Clear error flags
                bch->BHINT = 0xff;
                // 6 Byte-write all data block to BHDR.
                for (unsigned i = 0; i < 512; i++)
                    bch->BHDR = ((uint8_t *)&page_data[0])[block * 512 + i];
                if (start >= 0x0800)
                    for (unsigned i = 0; i < 3; i++)
                        bch->BHDR = ((uint8_t *)&oob_data[0])[block * 3 + i];
                // 7 Check BHINTS.ENCF bit or by enabling encoding finish interrupt.
                while (!(bch->BHINT & BIT(2)));
                // 8 When encoding finishes, read out the parity data in BHPARn.
                uart_puts("BCH ECC encoding: 0x");
                uart_puthex(bch->BHINT, 8);
                uart_puts(": 0x");
                uart_puthex(bch->BHPAR[0], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHPAR[1], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHPAR[2], 8);
                uart_puts(", 0x");
                uart_puthex(bch->BHPAR[3], 8);
                uart_puts("\r\n");
#endif
            }
        }

        for (unsigned i = 0; i < config.nand.page / 4; i++)
            *buf32++ = page_data[i];
        if (oob)
            for (unsigned i = 0; i < (config.nand.oob + 3) / 4; i++)
                *buf32++ = oob_data[i];
        start++;
    }

    uart_puts("ECC correctable: 0x");
    uart_puthex(ecc_count.correctable, 8);
    uart_puts(", uncorrectable: 0x");
    uart_puthex(ecc_count.uncorrectable, 8);
    uart_puts("\r\n");
}
