const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .mipsel,
        .cpu_model = .{ .explicit = &std.Target.mips.cpu.mips32 },
        .abi = .none,
        .os_tag = .freestanding,
    });

    const optimize = b.standardOptimizeOption(.{});

    const test_step = b.step("test", "Run tests");

    const Soc = enum {
        jz4740,
        jz4750,
        jz4755,
    };

    inline for (.{ Soc.jz4740, Soc.jz4750, Soc.jz4755 }) |soc| {
        const elf = b.addExecutable(.{
            .name = @tagName(soc),
            .root_module = b.createModule(.{
                .root_source_file = b.path("src/main.zig"),
                .target = target,
                .optimize = optimize,
            }),
        });

        elf.root_module.addAssemblyFile(b.path("src/startup.S"));
        elf.setLinkerScript(b.path(b.fmt("ld/{s}.ld", .{@tagName(soc)})));
        elf.entry = .{ .symbol_name = "_header" };

        const options = b.addOptions();
        options.addOption(Soc, "soc", soc);
        elf.root_module.addOptions("build_options", options);

        // Keep debug and frame pointers for debugging
        elf.root_module.strip = false;
        elf.root_module.omit_frame_pointer = false;
        // elf.link_data_sections = true;
        // elf.link_function_sections = true;
        elf.link_gc_sections = true;
        // LTO seems to cause compiler bugs
        // elf.want_lto = true;
        // No need for PIC & PIE
        elf.root_module.pic = false;
        elf.pie = false;

        const installElf = b.addInstallBinFile(
            elf.getEmittedBin(),
            b.fmt("{s}.elf", .{elf.out_filename}),
        );
        b.getInstallStep().dependOn(&installElf.step);

        // Binary image files
        const bin = elf.addObjCopy(.{ .format = .bin });
        const installBin = b.addInstallBinFile(
            bin.getOutput(),
            b.fmt("{s}.bin", .{elf.out_filename}),
        );
        b.getInstallStep().dependOn(&installBin.step);

        // Tests
        const elf_tests = b.addTest(.{ .root_module = elf.root_module });
        const run_elf_tests = b.addRunArtifact(elf_tests);
        test_step.dependOn(&run_elf_tests.step);
    }
}
