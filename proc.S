.section ".text.proc"

.global setup_stack
.global curr_core_id
.global curr_core_el
.global core_stack_base
.global switch_el

#define CORE_STACK_SIZE	0x1000

setup_stack:

    //get core id
    mrs x0, mpidr_el1
    and x0, x0, #0x3
 
    //core 0 does not need offset
    cbz	x0, 1f

    //get offset from base
    ldr x2, =CORE_STACK_SIZE
    mul x0, x0, x2

1:
    //get stack associated to core id
    adrp x1, __core0_stack
    //stack grows from high to low
    sub x1, x1, x0
    mov sp, x1

    //continue execution to main
    //for core 0
    cbnz x0, exec
    ret

exec:
    b __park_and_wait

//x0 = core number
//returns x0 = stack base
core_stack_base:
    //get offset from base
    ldr x2, =CORE_STACK_SIZE
    mul x0, x0, x2

1:
    //get stack associated to core id
    adrp x1, __core0_stack
    sub x0, x1, x0
    ret

curr_core_id:
    mrs x0, mpidr_el1
    and x0, x0, #0x3
    ret

curr_core_el:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2
    and x0, x0, #0x0F
    ret
