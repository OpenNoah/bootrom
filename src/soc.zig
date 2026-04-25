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

const Peripheral = switch (bopt.soc) {
    .jz4740 => enum {
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
    },
    .jz4755 => enum {
        UART0,
        UART1,
        UART2,
        UART3,
    },
};

pub fn peripheral(ph: Peripheral) struct {
    base: u32,
} {
    const base: u32 = pa_to_kseg1(switch (bopt.soc) {
        .jz4740 => switch (ph) {
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
            else => 0,
        },
        .jz4755 => switch (ph) {
            .UART0 => 0x10030000,
            .UART1 => 0x10031000,
            .UART2 => 0x10032000,
            .UART3 => 0x10033000,
        },
    });
    return .{
        .base = base,
    };
}
