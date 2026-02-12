// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/mutex.h>
#include <hobos/task.h>

mutex_t task_mutex = 0;
pid_t pid_cntr = 0;

/*
 * Create a clone of the parent task. If nothing there, then just create an
 * initial task.
 */
struct task *clone(struct task *task)
{
	struct task *new_task;
	
	new_task = kmalloc(sizeof(struct task))

	if (task)
		memcpy(new_task, task, sizeof(struct task));
}

pid_t get_pid(void)
{
	pid_t pid;

	acquire_mutex(&task_mutex);
	pid = pid_cntr++;
	release_mutex(&task_mutex);

	return pid;
}

