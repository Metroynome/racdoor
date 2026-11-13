#include <racdoor/exploit.h>

#include <assert.h>
#include <string.h>

static void checkHalfword(u32 log_address, u32 write_address)
{
	s32 index = helpLogIndex(log_address, write_address, 2);
	assert(index < 0);
	assert(log_address + ((u32) index << 1) == write_address);
}

int main(int argc, char** argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "unaligned") == 0)
			helpLogIndex(0x001a8ec8, 0x00120001, 2);
		else if (strcmp(argv[1], "unaligned-log") == 0)
			helpLogIndex(0x001a8ec9, 0x00120000, 2);
		else if (strcmp(argv[1], "width") == 0)
			helpLogIndex(0x001a8ec8, 0x00120000, 4);
		else if (strcmp(argv[1], "positive-byte") == 0)
			helpLogIndex(0x00100000, 0x00120000, 1);
		return 0;
	}

	/* Preserve the existing R&C1 instruction-byte target. */
	s32 rac_index = helpLogIndex(0x00141e08, 0x0012de29, 1);
	assert(rac_index == -0x13fdf);
	assert(0x00141e08 + (u32) rac_index == 0x0012de29);

	/* Exercise both subtraction wraparound and the sign-bit alias. These
	   destinations test arithmetic, not candidate executable hooks. */
	checkHalfword(0x001a8ec8, 0x00120000);
	checkHalfword(0x001a8ec8, 0x001c0000);
	checkHalfword(0x001a8ec8, 0x001a8ec8);
	checkHalfword(0x001a8ec8, 0);
	checkHalfword(0x001a8ec8, 0xfffffffe);
	return 0;
}
