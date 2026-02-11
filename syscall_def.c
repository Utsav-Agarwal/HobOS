// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/syscall_nr.h>
#include <hobos/syscall.h>
#include <hobos/kstdio.h>
#include <hobos/usr_ops.h>

void syscall_ni(struct syscall_meta *sys_m)
{
	kprintf("Syscall not implemented!\n");
}

/*
 * x1 - char to print
 */
void syscall_putc(struct syscall_meta *sys_m)
{
	char c = (char)sys_m->fn_args[1];

	putc(c);
}

/*
 * x1 - (char *) for 1st element of '\0' terminated string
 */
void syscall_puts(struct syscall_meta *sys_m)
{
	char *s = (char *)sys_m->fn_args[1];
	char buf[128];

	if (copy_from_user((void *)s, (void *)buf, 10) < 0)
		return;

	/* Ensure previous ops are complete */
	wmb();
	puts(buf);
}

/*
 * x1 - (char *) for 1st element of '\0' terminated string
 */
void syscall_panic(struct syscall_meta *sys_m)
{
	char *s = (char *)sys_m->fn_args[1];

	puts("Panic!:");
	puts(s);

	// TODO: trigger actual panic and register dump
	while (1)
		;
}
