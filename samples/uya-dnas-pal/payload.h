#ifndef UYA_DNAS_PAL_H
#define UYA_DNAS_PAL_H

#include <racdoor/util.h>

#define UYA_DNAS_WINDOW (0x004d55c8)

typedef enum uyaDnasResult {
	uyaDnasMismatch = 0,
	uyaDnasApplied = 1,
	uyaDnasAlreadyApplied = 2
} uyaDnasResult_e;

/* Match the PAL multiplayer instruction window before either byte write. */
static inline __attribute__((always_inline)) u32 uyaDnasApply(volatile u32 *code)
{
	volatile u8 *bytes = (volatile u8 *) code;
	if (code[0] != 0x2c420002 || code[1] != 0x1040000c ||
		code[2] != 0x24020007 || code[3] != 0x2402000a ||
		code[5] != 0x2402000b || code[6] != 0x10000004 ||
		code[8] != 0x56220032 || code[9] != 0xae570004 ||
		code[10] != 0x24020007)
		return uyaDnasMismatch;
	if (code[4] == 0x56220000 && code[7] == 0x24020005)
		return uyaDnasAlreadyApplied;
	if (code[4] != 0x56220003 || code[7] != 0x24020006)
		return uyaDnasMismatch;

	/* Horizon Launcher uses two 8-bit writes, not a full-instruction NOP. */
	bytes[0x10] = 0;
	bytes[0x1c] = 5;
	return uyaDnasApplied;
}

#endif
