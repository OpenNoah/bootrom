const std = @import("std");
const bopt = @import("build_options");

const soc = @import("soc.zig");
const mmio = @import("mmio.zig");

// // NAND busy JZ4740: PC30: Input, level trigger
// #define GPIOC_PINS_NAND_BUSY    30

// // NAND busy JZ4755: PC27: Input, level trigger
// #define GPIOC_PINS_NAND_BUSY    27

pub fn nand_busy_catch() void {
    // Not needed with current QEMU

    // // Low level trigger
    // gpc->DIR.C = BIT(GPIOC_PINS_NAND_BUSY);
    // // FLG.C is actually located at DAT.S
    // gpc->DAT.S = BIT(GPIOC_PINS_NAND_BUSY);
}

pub fn nand_busy_wait() void {
    // Not needed with current QEMU

    // // Wait for low level
    // while (!(gpc->FLG.D & BIT(GPIOC_PINS_NAND_BUSY)));
    // // High level trigger
    // gpc->DIR.S = BIT(GPIOC_PINS_NAND_BUSY);
    // // FLG.C is actually located at DAT.S
    // gpc->DAT.S = BIT(GPIOC_PINS_NAND_BUSY);
    // // Wait for high level
    // while (!(gpc->FLG.D & BIT(GPIOC_PINS_NAND_BUSY)));
}
