// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/mutex.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/task.h>

mutex_t task_mutex;
pid_t pid_cntr;

struct ctxt init_ctxt = {0};
struct task init_task = {
		.pid = 1,
		.ctxt = &init_ctxt,
};

struct task *get_curr_task(void)
{
	// TODO
	return &init_task;
}

void init_ktasks(void)
{
	// TODO
}

/*
 * Create a clone of the parent task. If nothing there, then just create an
 * initial task.
 */
struct task *clone(struct task *task)
{
	struct task *new_task;

	new_task = kmalloc(sizeof(*new_task));

	if (task)
		memcpy(new_task, task, sizeof(struct task));

	return new_task;
}

pid_t get_pid(void)
{
	pid_t pid;

	acquire_mutex(&task_mutex);
	pid = pid_cntr++;
	release_mutex(&task_mutex);

	return pid;
}

