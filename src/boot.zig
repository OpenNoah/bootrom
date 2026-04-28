const load_addr = 0x80000000;

var mem: [*]u8 = @ptrFromInt(load_addr);
var entry: u32 = load_addr;

pub fn init() void {
    mem = @ptrFromInt(load_addr);
    entry = load_addr;
}

pub fn load(data: []const u8) void {
    @memcpy(mem, data);
    mem += data.len;
}

pub fn debug(ph_uart: anytype) void {
    mem = @ptrFromInt(load_addr);
    for (0..8 * 1024 / 512) |bofs| {
        ph_uart.puts("SRAM 0x");
        ph_uart.puthex(bofs * 512, 8);
        ph_uart.puts(": 0x");
        var v: u32 = 0;
        v |= @as(u32, mem[bofs * 512 + 0]) << 0;
        v |= @as(u32, mem[bofs * 512 + 1]) << 8;
        v |= @as(u32, mem[bofs * 512 + 2]) << 16;
        v |= @as(u32, mem[bofs * 512 + 3]) << 24;
        ph_uart.puthex(v, 8);
        ph_uart.puts("\r\n");
    }
}

pub fn set_entry_offset(ofs: u32) void {
    entry = load_addr + ofs;
}

pub fn boot() noreturn {
    const jmp: *const fn () noreturn = @ptrFromInt(entry);
    jmp();
}
