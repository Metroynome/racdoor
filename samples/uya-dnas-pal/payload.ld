OUTPUT_ARCH(mips)
ENTRY(uyaDnasEntry)
SECTIONS
{
	. = 0;
	.text : { *(.text) *(.text.*) }
	.rodata : { *(.rodata*) }
	.data : { *(.data*) *(.sdata*) *(.got*) }
	.bss : { *(.bss*) *(.sbss*) *(COMMON) }
	/DISCARD/ : { *(.comment) *(.pdr) *(.mdebug*) *(.reginfo) }
}
