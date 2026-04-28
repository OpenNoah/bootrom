const std = @import("std");
const bopt = @import("build_options");

const soc = @import("soc.zig");
const mmio = @import("mmio.zig");
const gpio = @import("gpio.zig");
const boot = @import("boot.zig");

const is_busshare = true;

const IO = switch (bopt.soc) {
    .jz4740 => extern struct {
        NFCSR: mmio.Mmio(packed struct {
            NFE1: u1 = 0,
            NFCE1: u1 = 0,
            NFE2: u1 = 0,
            NFCE2: u1 = 0,
            NFE3: u1 = 0,
            NFCE3: u1 = 0,
            NFE4: u1 = 0,
            NFCE4: u1 = 0,
            _0: u24 = 0,
        }),
        _0: [0x100 - 0x54]u8,
        NFECCR: mmio.Mmio(u32),
        NFECC: mmio.Mmio(u32),
        NFPAR: [3]mmio.Mmio(u32),
        NFINTS: mmio.Mmio(u32),
        NFINTE: mmio.Mmio(u32),
        NFERR: [4]mmio.Mmio(u32),
    },
    .jz4755 => extern struct {
        NFCSR: mmio.Mmio(packed struct {
            NFE1: u1 = 0,
            NFCE1: u1 = 0,
            NFE2: u1 = 0,
            NFCE2: u1 = 0,
            NFE3: u1 = 0,
            NFCE3: u1 = 0,
            NFE4: u1 = 0,
            NFCE4: u1 = 0,
            _0: u24 = 0,
        }),
    },
    else => undefined,
};

pub fn peripheral(ph: soc.Peripheral) type {
    return struct {
        const bank = switch (ph) {
            .NAND_CS1 => 0,
            .NAND_CS2 => 1,
            .NAND_CS3 => 2,
            .NAND_CS4 => 3,
            else => undefined,
        };

        const Hw = soc.peripheral(IO, .NAND);

        const AddrPort = @as(*volatile u8, @ptrFromInt(soc.base_addr(ph) + switch (bopt.soc) {
            .jz4740 => 0x00010000,
            .jz4755 => if (is_busshare) 0x00010000 else 0x10,
            else => undefined,
        }));
        const CmdPort = @as(*volatile u8, @ptrFromInt(soc.base_addr(ph) + switch (bopt.soc) {
            .jz4740 => 0x00008000,
            .jz4755 => if (is_busshare) 0x00008000 else 0x08,
            else => undefined,
        }));
        const DataPort8 = soc.peripheral(u8, ph);
        const DataPort16 = soc.peripheral(u16, ph);
        const DataPort32 = soc.peripheral(u32, ph);

        row_cycles: u32 = undefined,
        page_size: u32 = undefined,
        oob_size: u32 = undefined,

        fn fce_assert() void {
            switch (bank) {
                0 => Hw.NFCSR.modify(.{ .NFCE1 = 1, .NFE1 = 1 }),
                1 => Hw.NFCSR.modify(.{ .NFCE2 = 1, .NFE2 = 1 }),
                2 => Hw.NFCSR.modify(.{ .NFCE3 = 1, .NFE3 = 1 }),
                3 => Hw.NFCSR.modify(.{ .NFCE4 = 1, .NFE4 = 1 }),
                else => {},
            }
        }

        fn fce_deassert() void {
            switch (bank) {
                0 => Hw.NFCSR.modify(.{ .NFCE1 = 0, .NFE1 = 1 }),
                1 => Hw.NFCSR.modify(.{ .NFCE2 = 0, .NFE2 = 1 }),
                2 => Hw.NFCSR.modify(.{ .NFCE3 = 0, .NFE3 = 1 }),
                3 => Hw.NFCSR.modify(.{ .NFCE4 = 0, .NFE4 = 1 }),
                else => {},
            }
        }

        pub fn init(self: *@This()) void {
            fce_deassert();

            // Reset and wait
            gpio.nand_busy_catch();
            CmdPort.* = 0xff;
            gpio.nand_busy_wait();

            if (bopt.soc == .jz4740) {
                self.row_cycles = 2;
                self.page_size = 2048;
                self.oob_size = 64;
            } else {
                // Read the first 12 bytes
                var header: [12]u8 = undefined;
                gpio.nand_busy_catch();
                CmdPort.* = 0x00;
                AddrPort.* = 0;
                AddrPort.* = 0;
                AddrPort.* = 0;
                AddrPort.* = 0;
                AddrPort.* = 0;
                CmdPort.* = 0x30;
                gpio.nand_busy_wait();
                for (0..header.len) |i|
                    header[i] = DataPort8.*;

                // Parse NAND boot info
                // const bus_width = if (header[0] == 0) 16 else 8;
                self.row_cycles = if (header[8] == 0) 2 else 3;
                self.page_size = if (header[9] == 0) 512 else if (header[10] == 0) 4096 else 2048;
                self.oob_size = 16 * self.page_size / 512;
            }
        }

        pub fn print_id(self: *@This(), ph_uart: anytype) void {
            _ = self;

            ph_uart.puts("NAND bank0 id: ");
            CmdPort.* = 0x90;
            AddrPort.* = 0x00;
            for (0..6) |_|
                ph_uart.puthex(DataPort8.*, 2);
            ph_uart.puts("\r\n");
        }

        pub fn load(self: *@This(), ph_uart: anytype) void {
            // Read 8k from NAND to 0x80000000
            var start: u32 = 0;
            var count: u32 = 8192 / self.page_size;
            const oob = false;
            _ = oob;
            ph_uart.puts("nand_read_pages: 0x");
            ph_uart.puthex(start, 8);
            ph_uart.puts(" + 0x");
            ph_uart.puthex(count, 8);
            ph_uart.puts("\r\n");

            while (count != 0) {
                gpio.nand_busy_catch();
                CmdPort.* = 0x00;
                AddrPort.* = (0 >> 0) & 0xff;
                AddrPort.* = (0 >> 8) & 0xff;
                AddrPort.* = @intCast((start >> 0) & 0xff);
                if (self.row_cycles >= 2)
                    AddrPort.* = @intCast((start >> 8) & 0xff);
                if (self.row_cycles >= 3)
                    AddrPort.* = @intCast((start >> 16) & 0xff);

                CmdPort.* = 0x30;
                gpio.nand_busy_wait();
                const bsize = 512;
                var buf: [bsize / 4]u32 = undefined;
                for (0..(self.page_size + bsize - 1) / bsize) |bofs| {
                    const blen = @min(bsize, self.page_size - bofs * bsize);
                    for (0..blen / 4) |i|
                        buf[i] = DataPort32.*;
                    boot.load(@ptrCast(buf[0 .. blen / 4]));
                }

                // var oob_data: [128 / 4]u32 = undefined;
                // for (0..self.oob_size / 4) |i|
                //     oob_data[i] = DataPort32.*;

                start += 1;
                count -= 1;
            }

            boot.set_entry_offset(switch (bopt.soc) {
                .jz4740 => 0x04,
                .jz4755 => 0x0c,
                else => undefined,
            });
        }
    };
}
