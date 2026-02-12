// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/mutex.h>
#include <hobos/task.h>

unsigned char pid_cntr = 0;

void clone(struct task *task)
{
}

unsigned char get_pid(void)
{
	return pid_cntr++;
}

