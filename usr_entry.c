// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/barrier.h>
#include <hobos/entry.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/lib/stdlib.h>
#include <hobos/mmu.h>
#include <hobos/kstdio.h>
#include <hobos/compiler_types.h>

/*
 * We essentially want to create an identity mapped translation table to enter userspace.
 * This will be similar to the kernel switch since we need to keep the page tables within the
 * realm of userspace as well.
 */
static void id_map_to_usr(u64 start, u64 end, struct page_table_desc *pt_desc)
{
	// add 3 page table addresses to current kernel table
	void *ret;
	u64 va = start;

	while (start < end) {
		ret = map_pa_to_va_pg(start, va, pt_desc,
				      PTE_FLAGS_USER_GENERIC, 0);
		if (ret != (void *)va) {
			kprintf("Failed at %x -> %x!(%x)\n", start, va, ret);
			return;
		}

		start += 0x1000;
		va += 0x1000;
	}
}

void usr_init(void)
{
	struct page_table_desc *usr_pt_desc;
	void *usr_pt;

	kprintf("\nUser memory range: [%x, %x] (%d KiB)\n",
		USR_INIT, USR_END, USR_SZ_KB);

	usr_pt_desc = kmalloc(sizeof(*usr_pt_desc));
	usr_pt_desc->level = 1;
	usr_pt_desc->pt = kmalloc(sizeof(u64) * 512);
	usr_pt = usr_pt_desc->pt;

	id_map_to_usr(USR_INIT, USR_END, usr_pt_desc);

	// TODO: Maybe assign a new page table?
	if (usr_pt)
		set_ttbr0_el1((u64)va_to_pa(usr_pt));
}

__section(".usr_entry") __noreturn void user_panic(void)
{
	while (1)
		;
}

__section(".usr_entry") static int mount_initrd(void)
{
	return 1;
}

__section(".usr_entry") __noreturn static void initrd_exec_init(void)
{
	while (1)
		;
}

__section(".usr_entry") __noreturn static void el0_default_init(void);

/*
 * TODO: Separate user and kernel space applications
 */
__section(".usr_entry") void usr_entry_pt(void)
{
	int ret = 0;
#ifdef	USR_INIT_APP
	void (*exec_init)(void) = (void(*)()) USR_INIT_APP;
#else
	void (*exec_init)(void) = el0_default_init;
#endif

	/*
	 * If we dont have a file system to work with, jump to the
	 * next userspace stub.
	 */
	ret = mount_initrd();
	if (!ret)
		initrd_exec_init();

	/* Make sure you load the correct instruction within the caches */
	mb();
	isb();
	exec_init();
	user_panic();
}

/*
 * Return EL0 entry function address with x0
 */
u64 get_usr_entry_pt(void)
{
	return (u64)usr_entry_pt;
}

u64 get_usr_end(void)
{
	return (USR_INIT + USR_SZ);
}

__section(".usr_mem") char el0_def_msg[] = "No init execution target address provided!\n";
__section(".usr_entry") __noreturn static void el0_default_init(void)
{
	asm volatile (
	    "mov x8, %0\n"
	    :
	    : "r"(0x2)
	    : "x8"
	);

	/*
	 * make sure x0 is not clobbered
	 */
	asm volatile (
	    "mov x1, %0\n"
	    "svc #0\n"
	    :
	    : "r"((u64)el0_def_msg)
	);

	user_panic();
}

