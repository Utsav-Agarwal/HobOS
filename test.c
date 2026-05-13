/* SPDX-License-Identifier: GPL-2.0-only */

#include <hobos/kstdio.h>
#include <hobos/test.h>
#include <hobos/sched.h>

//TODO: remove this before merge
char msg1[10] = "Hello\n";
char msg2[10] = "World\n";

static int print_msg(void *data)
{
	char *t_msg = (char *)data;

	for (int i=0; i<1000; i++)
		kprintf("%x %s", i, t_msg);
	
	return 5;
}

static int kthread_test(void)
{
	struct task *t1, *t2;

	kprintf("starting kthread test\n");
	t1 = kthread_create(print_msg, msg1);
	if (!t1)
		return -1;

	kthread_queue(t1);
	t2 = kthread_create(print_msg, msg2);
	if (!t2)
		return -1;

	kthread_queue(t2);
	return 0;
}

int kernel_test(void)
{
#if ENABLE_TEST
	return kthread_test();
#endif
	return 0;
}
