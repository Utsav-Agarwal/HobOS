// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/syscall.h>
#include <hobos/syscall_nr.h>
#include <hobos/kstdio.h>

extern syscall_t syscall_table[];

static struct syscall_meta get_syscall_meta(void)
{
	struct syscall_meta sys_m;

	asm volatile ("mov %0, x8" : "=r"(sys_m.syscall_nr));

	asm volatile ("mov %0, x0" : "=r"(sys_m.fn_args[0]));
	asm volatile ("mov %0, x1" : "=r"(sys_m.fn_args[1]));
	asm volatile ("mov %0, x2" : "=r"(sys_m.fn_args[2]));
	asm volatile ("mov %0, x3" : "=r"(sys_m.fn_args[3]));
	asm volatile ("mov %0, x4" : "=r"(sys_m.fn_args[4]));
	asm volatile ("mov %0, x5" : "=r"(sys_m.fn_args[5]));
	asm volatile ("mov %0, x6" : "=r"(sys_m.fn_args[6]));
	asm volatile ("mov %0, x7" : "=r"(sys_m.fn_args[7]));

	return sys_m;
}

/*
 * Simple syscall handler following the kernel ABI
 * x[0-7]	- arguments
 * x8		- syscall number
 */
void syscall_el0_common(void)
{
	struct syscall_meta sys_m = get_syscall_meta();
	u64 sys_nr = sys_m.syscall_nr;
	syscall_t syscall_fn;

	if (sys_nr >= SUPP_SYSCALL_NR)
		sys_nr = 0;

	syscall_fn = syscall_table[sys_nr];

	/* Ensure target function address was correctly updated */
	mb();
	isb();
	syscall_fn(&sys_m);
}
