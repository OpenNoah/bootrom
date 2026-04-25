pub fn prid() u32 {
    var v: u32 = 0;
    asm volatile ("mfc0 %[v], $15, 0"
        : [v] "=r" (v),
    );
    return v;
}

pub fn configs() [6]u32 {
    return .{
        asm volatile ("mfc0 %[v], $16, 0"
            : [v] "=r" (-> u32),
        ),
        asm volatile ("mfc0 %[v], $16, 1"
            : [v] "=r" (-> u32),
        ),
        asm volatile ("mfc0 %[v], $16, 2"
            : [v] "=r" (-> u32),
        ),
        asm volatile ("mfc0 %[v], $16, 3"
            : [v] "=r" (-> u32),
        ),
        asm volatile ("mfc0 %[v], $16, 4"
            : [v] "=r" (-> u32),
        ),
        asm volatile ("mfc0 %[v], $16, 5"
            : [v] "=r" (-> u32),
        ),
    };
}
