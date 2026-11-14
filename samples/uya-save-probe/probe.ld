OUTPUT_ARCH(mips)
ENTRY(probeEntry)
SECTIONS
{
	. = 0x001a9318;
	.text : { *(.text) }
	/DISCARD/ : { *(.comment) *(.pdr) *(.mdebug*) *(.reginfo) }
}
