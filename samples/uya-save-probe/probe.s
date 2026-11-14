.set noreorder
.set noat
.text
.globl probeEntry
.ent probeEntry
probeEntry:
	addiu $sp, $sp, -80
	sq $1, 0($sp)
	sq $2, 16($sp)
	sq $3, 32($sp)
	sq $4, 48($sp)
	sq $31, 64($sp)
	/* Restore the copy-loop branch before resuming it. */
	lui $1, 0x0014
	lui $2, 0x1440
	ori $2, $2, 0xfffa
	sw $2, -0x2d84($1)
	/* Restore the three overwritten block-39 words from saved metadata. */
	lui $3, 0x001b
	addiu $3, $3, -0x68e8
	lw $2, 0($3)
	sw $2, 0x266c($1)
	lw $2, 4($3)
	sw $2, 0x2670($1)
	lw $2, 8($3)
	sw $2, 0x2674($1)
	/* Make subsequent help writes in-bounds again. */
	lw $2, 12($3)
	lui $1, 0x001d
	sw $2, 0x5598($1)
	lui $2, 0x5052
	ori $2, $2, 0x4f42
	sw $2, 32($3)
	sync
	li $4, 0
	li $3, 0x64
	syscall
	li $4, 2
	li $3, 0x64
	syscall
	lq $1, 0($sp)
	lq $2, 16($sp)
	lq $3, 32($sp)
	lq $4, 48($sp)
	lq $31, 64($sp)
	/* The overwritten branch's delay-slot store has already run. */
	j 0x0013d268
	addiu $sp, $sp, 80
.end probeEntry
