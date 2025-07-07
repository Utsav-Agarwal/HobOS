.section ".text.proc"

.global setup_stack
.global curr_core_id

#define CORE_STACK_BASE	0x86000
#define CORE_STACK_SIZE	0x1000



//Check https://github.com/rsta2/circle/blob/927941cec722f530a23d0f636f03480dd9972056/lib/startup64.S#L106
//for RPI5
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
    ldr x1, =CORE_STACK_BASE
    sub x1, x1, x0
    mov sp, x1

    //continue execution to main
    //for core 0
    cbnz x0, exec
    ret

exec:
    b park_and_wait


curr_core_id:
    mrs x0, mpidr_el1
    and x0, x0, #0x3
    ret
