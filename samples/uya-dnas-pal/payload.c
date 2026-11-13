#include "payload.h"

/* Callable EE entry; the caller owns placement, scheduling and register saves. */
u32 uyaDnasEntry(void)
{
	u32 result = uyaDnasApply((volatile u32 *) UYA_DNAS_WINDOW);
	if (result == uyaDnasApplied) {
		/* PS2SDK FlushCache: syscall 0x64; writeback D-cache, invalidate I-cache. */
		__asm__ volatile (
			"sync\n\t"
			"li $4, 0\n\t"
			"li $3, 0x64\n\t"
			"syscall\n\t"
			"li $4, 2\n\t"
			"li $3, 0x64\n\t"
			"syscall\n\t"
			: : : "$2", "$3", "$4", "memory");
		return uyaDnasApplied;
	}
	return result;
}
