ENTRY(__start)

BSS_SIZE = 0xA000;

/*
 * We dont need too much, just enough until the scheduler takes over
 * since each task will have its own stack allocated to it
 */
CORE_STACK_SIZE = 0x1000;
PAGE_SIZE = 0x1000;
/* 1MB memory */
PHY_MEM_SIZE = PAGE_SIZE*256;

VMEM_OFFSET = 0xffffff8000000000;
SPIN_TABLE_BASE	= 0xD8;
KERNEL_START = 0x80000;
USER_START = 0x400000;

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = VMEM_OFFSET + KERNEL_START;
    /* For AArch64, use . = 0x80000; */
    .text : AT(KERNEL_START)
    {
        KEEP(*(.text.boot))
        KEEP(*(.text.proc))
        *(.text)
    }
    /* We need this so that we dont start in highmem */
    __start = LOADADDR(.text);
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
    . = ALIGN(PAGE_SIZE);

    __bss_start = .;
    .bss :
    {
	. += BSS_SIZE;
        bss = .;
        *(.bss)
    }
    __bss_end = .;
    . = ALIGN(PAGE_SIZE);

    /* SMP Stack */
    /* start translation tables here */
    /*. = CORE_STACK_BASE;*/

    /* stack grows high to low */
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

    /*
     * Leave some space for physical accessible memory
     * management metadata
     *
     * TODO: add a more dynamic size, the max entries can only
     * be: total size / min block size
     */
    .phymem_meta :
    {
	__phymem_meta_start = .;
	*(.__phymem_meta);
	. = . + (10*PAGE_SIZE);
	__phymem_meta_end = .;
    }
    . = ALIGN(PAGE_SIZE);

    /*
     * This is where heap starts, grows downwards
     */
    . = . + PHY_MEM_SIZE;
    __phymem_end = .;
    . = ALIGN(PAGE_SIZE);

    .kernel_pt :
    {
	__kernel_pt = .;
	*(.kernel_pt);
	. = . + (3*PAGE_SIZE);
    }
    . = ALIGN(PAGE_SIZE);
    /*
     * This is where heap starts, grows downwards
     */
    . = VMEM_OFFSET + USER_START;
    .usr_entry : AT(USER_START)
    {
	__usr_entry = .;
	*(.usr_entry);
    }
    . = ALIGN(PAGE_SIZE);
    .usr_mem :
    {
	*(.usr_mem);
    }
    . = ALIGN(PAGE_SIZE);

    /* We need something like a device tree for this */
    /* TODO: use ioremap for now */
    __rpi_spintable_base = VMEM_OFFSET + SPIN_TABLE_BASE;

    __end = . ;
}

