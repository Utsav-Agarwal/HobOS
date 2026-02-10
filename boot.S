/* SPDX-License-Identifier: GPL-2.0-only */

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
sync_exception_handler:
	bl kernel_panic
	eret

.macro ventry label
    .align 7
    b \label
.endm

dummy_vector:
    wfe
    b dummy_vector

/* We need to preserve caller program order */
.macro save_fplr
    sub sp, sp, #8*2
    stp x29, x30, [sp]
.endm

.macro restore_fplr
    ldp x29, x30, [sp]
    add sp, sp, #8*2
.endm

handle_el0_svc:
    save_fplr
    bl disable_global_interrupts
    bl do_syscall
    bl enable_global_interrupts
    restore_fplr
    eret

.align 11
vector_table:
    ventry sync_exception_handler
    ventry dummy_vector 
    ventry dummy_vector 
    ventry dummy_vector 
    
    //EL1h
    ventry dummy_vector 
    ventry handle_irq
    ventry dummy_vector 
    ventry dummy_vector

    ventry handle_el0_svc
