const std = @import("std");
const bopt = @import("build_options");

const cp0 = @import("cp0.zig");
const soc = @import("soc.zig");
const uart = @import("uart.zig");
const startup = @import("startup.zig");

comptime {
    @export(&startup.entry, .{ .name = "entry" });
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
    const ph_uart = uart.peripheral(soc.peripheral(switch (bopt.soc) {
        .jz4740 => .UART0,
        .jz4750 => .UART0,
        .jz4755 => .UART1,
    }));

    ph_uart.init();
    ph_uart.puts(std.fmt.comptimePrint(
        "\r\n*** bootrom {s} ***\r\n",
        .{@tagName(bopt.soc)},
    ));
    print_arch(ph_uart);

    while (true) {}
}
