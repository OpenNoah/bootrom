#pragma once

#include <stdint.h>

static inline uint32_t cp0_prid()
{
    uint32_t v;
    asm volatile ("mfc0 %0, $15, 0": "=r" (v));
    return v;
}

static inline void cp0_configs(uint32_t v[6])
{
    // Register 16 Select *
    asm volatile ("mfc0 %0, $16, 0": "=r" (v[0]));
    asm volatile ("mfc0 %0, $16, 1": "=r" (v[1]));
    asm volatile ("mfc0 %0, $16, 2": "=r" (v[2]));
    asm volatile ("mfc0 %0, $16, 3": "=r" (v[3]));
    asm volatile ("mfc0 %0, $16, 4": "=r" (v[4]));
    asm volatile ("mfc0 %0, $16, 5": "=r" (v[5]));
}
