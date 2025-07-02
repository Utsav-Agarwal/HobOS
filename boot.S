.section ".text.boot"


#define BIT(x) (1 << (x))

.global _start
.global __heap_start
.global __heap_size
.global __heap_end

#define SMPEN	BIT(6)

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    //set heap address
4:  ldr x0, =__heap_start
    mov w1, #0x20000000
    str w1, [x0]
    
    ldr x0, =__heap_size
    mov w1, #0x4000
    str w1, [x0]

    //calculate the value for __heap_end
    ldr x1, =__heap_end
    ldr x2, =__heap_start
    ldr x3, =__heap_size

    add x2, x3, x0
    str x0, [x1]

    // jump to C code, should not return
5:  bl      main
    // for failsafe, halt this core too
    b       1b
