#pragma once

#include <stdint.h>

#define _TOKENPASTE(x, y)	x ## y
#define TOKENPASTE(x, y)	_TOKENPASTE(x, y)

#define MHZ(n)			((n) * 1000000)

#define PA_TO_KSEG0(v)	(0x80000000 + (v))	// Unmapped, cachable
#define PA_TO_KSEG1(v)	(0xa0000000 + (v))	// Unmapped, uncacheable

#define DIV_CEIL(a, b)	(((a) + (b) - 1) / (b))
#define BIT(n)			(1ul << (n))

#define RESERVED_VAR	TOKENPASTE(_RESERVED_, __LINE__)

const char *get_hex_u32(const char *s, uint32_t *pv);

static inline uint32_t kseg0_to_kseg1(const void *p)
{
	return (uint32_t)p - 0x80000000 + 0xa0000000;
}

static inline uint32_t kseg0_to_pa(const void *p)
{
	return (uint32_t)p - 0x80000000;
}

static inline uint32_t kseg1_to_pa(const void *p)
{
	return (uint32_t)p - 0xa0000000;
}

void *alloc(uint32_t size);
