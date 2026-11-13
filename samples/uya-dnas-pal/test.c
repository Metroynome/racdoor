#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "payload.h"

static const u32 original[] = {
	0x2c420002, 0x1040000c, 0x24020007, 0x2402000a,
	0x56220003, 0x2402000b, 0x10000004, 0x24020006,
	0x56220032, 0xae570004, 0x24020007
};

int main(void)
{
	u32 storage[13], before[13];
	u32 *code = storage + 1;
	u32 i;
	memset(storage, 0xa5, sizeof(storage));
	memcpy(code, original, sizeof(original));
	memcpy(before, storage, sizeof(storage));
	assert(uyaDnasApply(code) == uyaDnasApplied);
	((u8 *) (before + 1))[0x10] = 0;
	((u8 *) (before + 1))[0x1c] = 5;
	assert(memcmp(storage, before, sizeof(storage)) == 0);
	assert(uyaDnasApply(code) == uyaDnasAlreadyApplied);
	assert(memcmp(storage, before, sizeof(storage)) == 0);

	for (i = 0; i < sizeof(original) / sizeof(original[0]); i++) {
		memcpy(code, original, sizeof(original));
		code[i] ^= 0x80000000;
		memcpy(before, storage, sizeof(storage));
		assert(uyaDnasApply(code) == uyaDnasMismatch);
		assert(memcmp(storage, before, sizeof(storage)) == 0);
	}
	for (i = 0; i < 2; i++) {
		memcpy(code, original, sizeof(original));
		if (i == 0) code[4] = 0x56220000;
		else code[7] = 0x24020005;
		memcpy(before, storage, sizeof(storage));
		assert(uyaDnasApply(code) == uyaDnasMismatch);
		assert(memcmp(storage, before, sizeof(storage)) == 0);
	}
	puts("PAL DNAS: exact byte changes, idempotence, overlay and partial-patch rejection passed.");
	return 0;
}
