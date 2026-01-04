.section ".text.boot"

.global _start

_start:

    bl setup_stack

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =BSS_SIZE
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b
    
4: 
    bl jump_to_EL1

    mov sp, x0
    adr	x0, vector_table 
    msr vbar_el1, x0
    isb

    // jump to C code, should not return
5:  bl      main
    // for failsafe, halt this core too
    b       5b


.align 11
vector_table:
	b sync_exception_handler
	b 0x200
	b 0x200

.align 11
sync_exception_handler:
	bl kernel_panic
	eret
