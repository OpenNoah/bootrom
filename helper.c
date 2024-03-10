#include "helper.h"

extern char __stack_end__;

const char *get_hex_u32(const char *s, uint32_t *pv)
{
	uint32_t v = 0;
	for (;;) {
		char c = *s;
		if (c == '\0')
			break;
		else if (c >= '0' && c <= '9')
			v = (v << 4) | (c - '0');
		else if (c >= 'a' && c <= 'f')
			v = (v << 4) | (c - 'a' + 0xa);
		else if (c >= 'A' && c <= 'F')
			v = (v << 4) | (c - 'A' + 0xa);
		else if (c == 'x' || c == 'X')
			;
		else
			break;
		s++;
	}
	*pv = v;
	return s;
}

void *alloc(uint32_t size)
{
	extern char __alloc_start__;
	static void *ptr = &__alloc_start__;
	void *p = ptr;
	ptr += size;
	return p;
}
