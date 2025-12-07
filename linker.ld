ENTRY(_start)

BSS_SIZE = 0x4000;

CORE_STACK_SIZE = 0x1000;
PAGE_SIZE = 0x1000;

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x80000;
    /* For AArch64, use . = 0x80000; */
    __start = .;
    .text :
    {
        KEEP(*(.text.boot))
        KEEP(*(.text.proc))
        *(.text)
    }
    . = ALIGN(PAGE_SIZE); 
    .rodata :
    {
        *(.rodata)
    }

    . = ALIGN(PAGE_SIZE); 
    __data_start = .;
    .data :
    {
        *(.data)
    }

    __bss_start = .;
    .bss :
    {
	. += BSS_SIZE;
        bss = .;
        *(.bss)
    }
    __bss_end = .;


    /* NOTE: ARM stack grows downwards */

    /* SMP Stack */
    /* start translation tables here */
    /*. = CORE_STACK_BASE;*/
    .core0_stack :
    {
	. = . + CORE_STACK_SIZE;
	*(.core0_stack)
	__core0_stack = .;	
    }
    . = ALIGN(PAGE_SIZE); 

    .core1_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core1_stack)
        __core1_stack = .;
    }
    . = ALIGN(PAGE_SIZE); 
    
    .core2_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core2_stack)
        __core2_stack = .;
    }
    . = ALIGN(PAGE_SIZE); 
    
    .core3_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core3_stack)
        __core3_stack = .;
    }
    . = ALIGN(PAGE_SIZE); 
   
    /* Leave some buffer for misc
     * components such as exception vector tables
     */
    . += 0x1000;

    __end = . ;
}

