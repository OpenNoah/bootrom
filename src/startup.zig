const std = @import("std");
const root = @import("root");

extern var __bss_start__: u32;
extern var __bss_end__: u32;

extern var __data_load__: u32;
extern var __data_start__: u32;
extern var __data_end__: u32;

pub fn entry() callconv(.c) noreturn {
    const bss = @as([*]u32, @ptrCast(&__bss_start__))[0 .. &__bss_end__ - &__bss_start__];
    @memset(bss, 0);
    const data = @as([*]u32, @ptrCast(&__data_start__))[0 .. &__data_end__ - &__data_start__];
    @memcpy(data, @as([*]u32, @ptrCast(&__data_load__)));
    root.main();
    while (true) {}
}
