const std = @import("std");

const soc = @import("soc.zig");
const mmio = @import("mmio.zig");

const IO = extern struct {
    CTRL: mmio.Mmio(packed struct {
        CLOCK_CONTROL: u2 = 0,
        START_OP: u1 = 0,
        RESET: u1 = 0,
        STOP_READWAIT: u1 = 0,
        START_READWAIT: u1 = 0,
        EXIT_TRANSFER: u1 = 0,
        EXIT_MULTIPLE: u1 = 0,
        _0: u6 = 0,
        SEND_AS_CCSD: u1 = 0,
        SEND_CCSD: u1 = 0,
    }),
    _0: u16,

    STAT: mmio.Mmio(packed struct {
        TIME_OUT_READ: u1,
        TIME_OUT_RES: u1,
        CRC_WRITE_ERROR: u2,
        CRC_READ_ERROR: u1,
        CRC_RES_ERR: u1,
        DATA_FIFO_EMPTY: u1,
        DATA_FIFO_FULL: u1,
        CLK_EN: u1,
        IS_READWAIT: u1,
        DATA_FIFO_AFULL: u1,
        END_CMD_RES: u1,
        DATA_TRAN_DONE: u1,
        PRG_DONE: u1,
        SDIO_INT_ACTIVE: u1,
        IS_RESETTING: u1,
        _0: u15,
        AUTO_CMD_DONE: u1,
    }),

    CLKRT: mmio.Mmio(packed struct {
        CLK_RATE: u3 = 0,
        _0: u13 = 0,
    }),
    _1: u16,

    CMDAT: mmio.Mmio(packed struct {
        RESPONSE_FORMAT: u3 = 0,
        DATA_EN: u1 = 0,
        WRITE_READ: u1 = 0,
        STREAM_BLOCK: u1 = 0,
        BUSY: u1 = 0,
        INIT: u1 = 0,
        DMA_EN: u1 = 0,
        BUS_WIDTH: u2 = 0,
        STOP_ABORT: u1 = 0,
        TTRG: u2 = 0,
        RTRG: u2 = 0,
        SEND_AS_STOP: u1 = 0,
        SDIO_PRDT: u1 = 0,
        _0: u12 = 0,
        READ_CEATA: u1 = 0,
        CCS_EXPECTED: u1 = 0,
    }),

    RESTO: mmio.Mmio(packed struct {
        RES_TO: u8 = 0,
        _0: u8 = 0,
    }),
    _2: u16,

    RDTO: mmio.Mmio(packed struct {
        READ_TO: u32 = 0,
    }),

    BLKLEN: mmio.Mmio(u16),
    _3: u16,
    NOB: mmio.Mmio(u16),
    _4: u16,
    SNOB: mmio.Mmio(u16),
    _5: u16,
    IMASK: mmio.Mmio(u32),
    IREG: mmio.Mmio(u16),
    _6: u16,

    CMD: mmio.Mmio(packed struct {
        CMD_INDEX: u6 = 0,
        _0: u2 = 0,
    }),
    _7: u8,
    _8: u16,

    ARG: mmio.Mmio(u32),
    RES: mmio.Mmio(u16),
    _9: u16,
    RXFIFO: mmio.Mmio(u32),
    TXFIFO: mmio.Mmio(u32),

    LPM: mmio.Mmio(packed struct {
        LPM: u1 = 0,
        _0: u7 = 0,
    }),
};

pub fn peripheral(ph: soc.Peripheral) type {
    return struct {
        rca: u32 = 0,

        const Hw = soc.peripheral(IO, ph);

        const CmdR1 = enum(u6) {
            GO_IDLE_STATE = 0,
            SELECT_CARD = 7,
            SEND_STATUS = 13,
            SET_BLOCKLEN = 16,
            READ_SINGLE_BLOCK = 17,
            READ_MULTIPLE_BLOCK = 18,
            APP_CMD = 55,
        };

        const CmdR2 = enum(u6) {
            ALL_SEND_CID = 2,
        };

        const CmdR6 = enum(u6) {
            SEND_RELATIVE_ADDR = 3,
        };

        const CmdR7 = enum(u6) {
            SEND_IF_COND = 8,
        };

        const AcmdR3 = enum(u6) {
            SD_SEND_OP_COND = 41,
        };

        fn sd_cmd(self: *@This(), cmd: anytype, arg: u32) switch (@TypeOf(cmd)) {
            CmdR2 => u128,
            else => u32,
        } {
            const resp_format = switch (@TypeOf(cmd)) {
                CmdR1 => 1,
                CmdR2 => 2,
                AcmdR3 => 3,
                CmdR6 => 6,
                CmdR7 => 7,
                else => 1,
            };
            _ = switch (@TypeOf(cmd)) {
                AcmdR3 => sd_cmd(self, CmdR1.APP_CMD, 0),
                else => {},
            };
            Hw.CMD.write(.{ .CMD_INDEX = @intFromEnum(cmd) });
            Hw.CMDAT.write(.{ .RESPONSE_FORMAT = resp_format });
            Hw.ARG.write(arg);
            Hw.CTRL.write(.{ .START_OP = 1 });
            while (Hw.STAT.read().END_CMD_RES != 1) {}

            const resp_len = switch (@TypeOf(cmd)) {
                CmdR2 => 136,
                else => 48,
            };
            var v: @Int(.unsigned, resp_len) = 0;
            for (0..resp_len / 16) |_| {
                v <<= 16;
                v |= Hw.RES.read();
            }
            return @intCast(v >> 8);
        }

        fn sd_read(self: *@This(), addr: u32, buf: []u8) u32 {
            Hw.CMD.write(.{ .CMD_INDEX = @intFromEnum(CmdR1.READ_SINGLE_BLOCK) });
            Hw.CMDAT.write(.{ .WRITE_READ = 0, .DATA_EN = 1, .RESPONSE_FORMAT = 1 });
            Hw.ARG.write(addr);
            Hw.NOB.write(1);
            Hw.CTRL.write(.{ .START_OP = 1 });
            while (Hw.STAT.read().END_CMD_RES != 1) {}

            const resp_len = 48;
            var v: @Int(.unsigned, resp_len) = 0;
            for (0..resp_len / 16) |_| {
                v <<= 16;
                v |= Hw.RES.read();
            }

            _ = self;

            // _ = buf;
            for (0..buf.len / 4) |ofs| {
                const d = Hw.RXFIFO.read();
                buf[ofs * 4 + 3] = @intCast((d >> 24) & 0xff);
                buf[ofs * 4 + 2] = @intCast((d >> 16) & 0xff);
                buf[ofs * 4 + 1] = @intCast((d >> 8) & 0xff);
                buf[ofs * 4 + 0] = @intCast((d >> 0) & 0xff);
            }

            return @intCast(v >> 8);
        }

        pub fn init(self: *@This()) void {
            _ = self.sd_cmd(CmdR1.GO_IDLE_STATE, 0);
            // The HCS (Host Capacity Support) bit set to 1 indicates that the host supports SDHC or SDXC Card
            // XPC set to Maximum Performance
            // OCR set to support 2.7-3.6V
            _ = self.sd_cmd(AcmdR3.SD_SEND_OP_COND, 0x50ff8000);
            _ = self.sd_cmd(CmdR2.ALL_SEND_CID, 0);
            const v = sd_cmd(self, CmdR6.SEND_RELATIVE_ADDR, 0);
            self.rca = v & 0xffff0000;
            _ = self.sd_cmd(CmdR1.SELECT_CARD, self.rca);
            _ = self.sd_cmd(CmdR1.SEND_STATUS, self.rca);
            _ = self.sd_cmd(CmdR1.SET_BLOCKLEN, 512);
            Hw.BLKLEN.write(512);
        }

        pub fn boot(self: *@This(), ph_uart: anytype) void {
            var buf: [512]u8 = undefined;
            _ = self.sd_read(0, &buf);
            ph_uart.puts("SD 0x");
            var v: u32 = 0;
            v |= @as(u32, buf[0]) << 0;
            v |= @as(u32, buf[1]) << 8;
            v |= @as(u32, buf[2]) << 16;
            v |= @as(u32, buf[3]) << 24;
            ph_uart.puthex(v, 8);
            ph_uart.puts("\r\n");
            _ = self.sd_read(1, &buf);
            ph_uart.puts("SD 0x");
            v = 0;
            v |= @as(u32, buf[0]) << 0;
            v |= @as(u32, buf[1]) << 8;
            v |= @as(u32, buf[2]) << 16;
            v |= @as(u32, buf[3]) << 24;
            ph_uart.puthex(v, 8);
            ph_uart.puts("\r\n");
        }
    };
}
