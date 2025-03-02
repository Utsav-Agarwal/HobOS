//Aarch64

//keep this in the first portion of the binary
.section ".text.boot"

//Make _start global
.globl _start
	.org 0x80000


//Entry point for kernel.
//Sleeping cores(1,2,3) at 0xE0/E8/F0, only core 0 runs by default
// x0 -> 32 bit pointer to DTB in memory (primary core only) / 0 (secondary cores)
// x1 -> 0
// x2 -> 0
// x3 -> 0
// x4 -> 32 bit kernel entry point, _start location
//NOTE: X* W* = general purpose registers
_start:
	//set the stack pointer
	ldr x5, =_start
	mov sp, x5

	//clear bss
	ldr x5, =__bss_start
	ldr w6, =__bss_size

clear_bss:
	cbz w6, kernel_noret
	str xzr, [x5], #8
	sub w6, w6, #1
	cbnz w6, clear_bss

//should not return
kernel_noret:
	bl kernel_main

//failsafe - something went wrong
//halt:
//	wfe
//	b halt
