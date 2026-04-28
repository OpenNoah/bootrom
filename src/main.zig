const std = @import("std");
const bopt = @import("build_options");

const soc = @import("soc.zig");
const cp0 = @import("cp0.zig");
const uart = @import("uart.zig");
const msc = @import("msc.zig");
const nand = @import("nand.zig");
const boot = @import("boot.zig");

comptime {
    @export(&@import("startup.zig").entry, .{ .name = "entry" });
}

fn print_arch(ph_uart: anytype) void {
    ph_uart.puts("Processor ID: 0x");
    ph_uart.puthex(cp0.prid(), 8);
    ph_uart.puts("\r\n");

    for (cp0.configs(), 0..) |v, i| {
        ph_uart.puts("CP0 Config ");
        ph_uart.puthex(i, 1);
        ph_uart.puts(": 0x");
        ph_uart.puthex(v, 8);
        ph_uart.puts("\r\n");
    }
}

pub fn main() noreturn {
    const ph_uart = uart.peripheral(switch (bopt.soc) {
        .jz4740 => .UART0,
        .jz4750 => .UART0,
        .jz4755 => .UART0,
    });

    ph_uart.init();
    ph_uart.puts(std.fmt.comptimePrint(
        "\r\n*** bootrom {s} ***\r\n",
        .{@tagName(bopt.soc)},
    ));
    print_arch(ph_uart);

    boot.init();
    if (bopt.soc == .jz4740) {
        // Load 8KiB data and boot from NAND
        var ph_nand = nand.peripheral(.NAND_CS1){};
        ph_nand.init();
        ph_nand.print_id(ph_uart);
        ph_nand.load(ph_uart);
    } else if (bopt.soc == .jz4750) {
        // Load 8KiB data and boot from MSC0
        var ph_msc0 = msc.peripheral(.MSC0){};
        ph_msc0.init();
        ph_msc0.load();
    }
    boot.debug(ph_uart);
    boot.boot();
}
