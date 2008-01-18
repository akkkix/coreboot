/* For starting linuxBIOS in protected mode */

#include <arch/rom_segs.h>

/* 	.section ".rom.text" */
	.code32

	.align	4
.globl gdtptr

	/* This is the gdt for ROMCC/ASM part of LinuxBIOS.
	 * It is different from the gdt in GCC part of LinuxBIOS
	 * which is defined in c_start.S */
gdt:
gdtptr:
	.word	gdt_end - gdt -1 /* compute the table limit */
	.long	gdt		 /* we know the offset */
	.word	0

	/* selgdt 0x08, flat code segment */
	.word	0xffff, 0x0000		
	.byte	0x00, 0x9b, 0xcf, 0x00 /* G=1 and 0x0f, So we get 4Gbytes for limit */	

	/* selgdt 0x10,flat data segment */
	.word	0xffff, 0x0000		
	.byte	0x00, 0x93, 0xcf, 0x00

gdt_end:
	

/*
 *	When we come here we are in protected mode. We expand 
 *	the stack and copies the data segment from ROM to the
 *	memory.
 *
 *	After that, we call the chipset bootstrap routine that
 *	does what is left of the chipset initialization. 
 *
 *	NOTE aligned to 4 so that we are sure that the prefetch
 *	cache will be reloaded.
 */
	.align	4
.globl protected_start
protected_start:

	lgdt	%cs:gdtptr
	ljmp	$ROM_CODE_SEG, $__protected_start
	
__protected_start:
	/* Save the BIST value */
	movl	%eax, %ebp

	intel_chip_post_macro(0x10)	/* post 10 */

	movw	$ROM_DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	%ax, %fs
	movw	%ax, %gs

	/* Restore the BIST value to %eax */
	movl	%ebp, %eax
