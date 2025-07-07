ENTRY(_start)


BSS_SIZE = 0x4000;

CORE_STACK_BASE = 0x86000;
CORE_STACK_SIZE = 0x1000;

PAGE_SIZE = 0x4000;

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x80000;
    /* For AArch64, use . = 0x80000; */
    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        KEEP(*(.text.proc))
        *(.text)
    }
    . = ALIGN(PAGE_SIZE); /* align to page size */
    __text_end = .;

    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    . = ALIGN(PAGE_SIZE); /* align to page size */
    __rodata_end = .;

    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(PAGE_SIZE); /* align to page size */
    __data_end = .;

    __bss_start = .;
    .bss :
    {
	. += BSS_SIZE;
        bss = .;
        *(.bss)
    }
    . = ALIGN(PAGE_SIZE); /* align to page size */
    __bss_end = .;


    /* SMP Stack */
    /* NOTE: ARM stack grows downwards */
   
    . = CORE_STACK_BASE;
    .core0_stack :
    {
	. = . + CORE_STACK_SIZE;
	*(.core0_stack)
	__core0_stack = .;	
    }
    . = ALIGN(PAGE_SIZE); /* align to page size */

    .core1_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core1_stack)
        __core1_stack = .;
    }
    . = ALIGN(PAGE_SIZE); /* align to 16 bytes */
    
    .core2_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core2_stack)
        __core2_stack = .;
    }
    . = ALIGN(PAGE_SIZE); /* align to 16 bytes */
    
    .core3_stack :
    {
        . = . + CORE_STACK_SIZE;
        *(.core3_stack)
        __core3_stack = .;
    }
    . = ALIGN(PAGE_SIZE); /* align to 16 bytes */

    __end = .;
}

