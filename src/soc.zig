const std = @import("std");
const bopt = @import("build_options");

// Unmapped, cachable
fn pa_to_kseg0(v: u32) u32 {
    return 0x80000000 + v;
}

// Unmapped, uncacheable
fn pa_to_kseg1(v: u32) u32 {
    return 0xa0000000 + v;
}

pub const Peripheral = switch (bopt.soc) {
    .jz4740 => enum {
        NAND,
        NAND_CS1,
        NAND_CS2,
        NAND_CS3,
        NAND_CS4,
        UART0,
        UART1,
        UART2,
        UART3,
    },
    .jz4750 => enum {
        CGU,
        LCD,
        EMC,
        SRAM,
        NAND,
        SDRAM,
        BCH,
        UART0,
        UART1,
        UART2,
        UART3,
        I2C,
        MSC0,
        MSC1,
    },
    .jz4755 => enum {
        UART0,
        UART1,
        UART2,
        UART3,
    },
};

pub fn base_addr(ph: Peripheral) u32 {
    return pa_to_kseg1(switch (bopt.soc) {
        .jz4740 => switch (ph) {
            .NAND => 0x13010050,
            .NAND_CS1 => 0x18000000,
            .NAND_CS2 => 0x14000000,
            .NAND_CS3 => 0x0c000000,
            .NAND_CS4 => 0x08000000,
            .UART0 => 0x10030000,
            .UART1 => 0x10031000,
            .UART2 => 0x10032000,
            .UART3 => 0x10033000,
        },
        .jz4750 => switch (ph) {
            .CGU => 0x10000000,
            .LCD => 0x13050000,
            .UART0 => 0x10030000,
            .UART1 => 0x10031000,
            .UART2 => 0x10032000,
            .UART3 => 0x10033000,
            .MSC0 => 0x10021000,
            .MSC1 => 0x10022000,
            else => 0,
        },
        .jz4755 => switch (ph) {
            .UART0 => 0x10030000,
            .UART1 => 0x10031000,
            .UART2 => 0x10032000,
            .UART3 => 0x10033000,
        },
    });
}

pub fn peripheral(ph_type: type, ph: Peripheral) *volatile ph_type {
    return @ptrFromInt(base_addr(ph));
}
