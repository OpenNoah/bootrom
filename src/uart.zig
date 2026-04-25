const mmio = @import("mmio.zig");

pub const IO = extern struct {
    r0: extern union {
        URBR: mmio.Mmio(u32),
        UTHR: mmio.Mmio(u32),
        UDLLR: mmio.Mmio(u32),
    },
    r1: extern union {
        UIER: mmio.Mmio(u32),
        UDLHR: mmio.Mmio(u32),
    },
    r2: extern union {
        UIIR: mmio.Mmio(u32),
        UFCR: mmio.Mmio(u32),
    },
    ULCR: mmio.Mmio(u32),
    UMCR: mmio.Mmio(u32),
    ULSR: mmio.Mmio(u32),
    UMSR: mmio.Mmio(u32),
    USPR: mmio.Mmio(u32),
    ISR: mmio.Mmio(u32),
    UMR: mmio.Mmio(u32),
    UACR: mmio.Mmio(u32),
};

fn io(base: u32) *volatile IO {
    return @ptrFromInt(base);
}

const Hw = struct {
    hw: *volatile IO,

    pub fn init(self: Hw) void {
        // Disable UART
        self.hw.r2.UFCR.write(0);
        // DLAB = 1, 8-bit
        self.hw.ULCR.write(0b10000011);
        // DLAB = 0, 8-bit
        self.hw.ULCR.write(0b00000011);
        // Enable UART, clear FIFO
        self.hw.r2.UFCR.write(0b00010111);
    }

    pub fn putc(self: Hw, c: u8) void {
        // TEMP
        while (self.hw.ULSR.read() & (1 << 6) == 0) {}
        self.hw.r0.UTHR.write(c);
    }

    pub fn puts(self: Hw, str: []const u8) void {
        for (str) |c|
            self.putc(c);
    }

    pub fn puthex(self: Hw, v: anytype, w: u32) void {
        for (0..w) |i| {
            const fv: u8 = @intCast((v >> @intCast(4 * (w - 1 - i))) & 0x0f);
            const c: u8 = if (fv < 10) fv + '0' else fv + 'a' - 10;
            self.putc(c);
        }
    }
};

pub fn peripheral(ph: anytype) Hw {
    return .{
        .hw = io(ph.base),
    };
}
