// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/mutex.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/task.h>
#include <hobos/sched.h>

mutex_t pid_mutex;
pid_t pid_cntr;

/*
 *
 * Essentially what we want is a 3 task initial structure:
 *
 * PID	Job
 *
 * 0	idle process, run when nothing to do, just sits in a while loop
 * 1	init kernel and then jump to userspace
 * 2	init kernel threading daemon. This is responsible for handling
 *	all kthread_create() calls
 *
 */

/*
 * This is the process that will eventually become the idle process
 */
struct ctxt init_ctxt;
struct task init_task = {
		.pid = 0,
		.ctxt = &init_ctxt,
};

pid_t assign_new_pid(void)
{
	pid_t pid;

	acquire_mutex(&pid_mutex);
	pid = pid_cntr++;
	release_mutex(&pid_mutex);

	return pid;
}

/*
 * Create a clone of the parent task. If nothing there, then just create an
 * initial task.
 */
struct task *clone(struct task *task)
{
	struct task *new_task;
	struct ctxt *new_ctxt;

	if (task) {
		new_task = kmalloc(sizeof(*new_task));
		new_ctxt = kmalloc(sizeof(*new_ctxt));
		if (!new_task)
			return 0;
	} else {
		return 0;
	}

	memcpy(new_task, task, sizeof(*new_task));
	new_task->pid = assign_new_pid();
	new_ctxt->sp = new_task->stack;
	new_task->ctxt = new_ctxt;
	return new_task;
}

/*
 * sp_el0 is unused when executing in kernel. This means we can
 * leverage that to store the current executing task without it being a memory
 * operation - i.e, no load/store needed.
 */
void set_curr_task(struct task *t)
{
	asm volatile("msr sp_el0, %0"::"r"(t));
}

inline struct task *get_curr_task(void)
{
	void *t;

	//its fine to cache this value. We dont change it too much
	asm("mrs %0, sp_el0" : "=r"(t));

	return (struct task *)t;
}

/* Make sure the init is set to idle if no task */
void kthread_init(void)
{
	set_curr_task(&init_task);
}

struct task *kthread_create(int (*thread_fn)(void *data), void *data)
{
	struct task *t, *new_t;

	t = get_curr_task();
	new_t = clone(t);
	if (!new_t)
		return 0;

	new_t->pc = thread_fn;
	new_t->running = 0;
	new_t->data = data;

	return new_t;
}

/*
 * We need this to exploit the callee saved regs and update the processor
 * state accordingly.
 */
__noreturn void kthread_ret_from_fork(void)
{
	int (*thread_fn)(void *data);
	void *data;
	int ret;

	asm volatile("mov %0, x20" : "=r"(data));
	asm volatile("mov %0, x19" : "=r"(thread_fn));

	ret = thread_fn(data);
	kprintf("Thread returned with ret: %x\n", ret);

	//TODO: free task
	//kthread_free_resources();
	schedule();
	//TODO: run idle proc if nothing
	while (1)
		;
}

__noreturn void kthread_init_stack_and_run(struct task *t)
{
	/*
	 * We want the task to start with its own stack and in doing so,
	 * we dont want the current stack to be altered too much and lose
	 * control - essentially, each struct task should be a self contained
	 * instance to run anything and everything without disturbing the
	 * current/parent stack.
	 *
	 * So we create a fake stack frame which tells the processor where to
	 * "ret" to and start execution for the function.
	 *
	 *
	 * (Low mem)
	 * --------------
	 * SP
	 * --------------
	 * Callee regs [x19-x30]
	 * --------------
	 * (High mem)
	 */
	struct ctxt *ctxt = t->ctxt;

	/*
	 * Since we cant use x0-x18 (caller saved regs, can be overwritten)
	 * we need to designate one of the regs to contain the data.
	 */
	ctxt->x[0] = (u64)(t->pc);
	ctxt->x[1] = (u64)(t->data);
	ctxt->x[11] = (u64)(kthread_ret_from_fork);

	/* Ensure that all previous memory operations have been completed
	 * since ctxt read/wrties are load/stores
	 */
	wmb();

	/* Start executing */
	resume_ctxt(t);

	/* fail safe */
	while (1)
		;
}

int kthread_start(struct task *t)
{
	if (!t)
		return -1;

	/* We only care about starting a task, not resuming it */
	if (t->running)
		return 0;

	kthread_init_stack_and_run(t);
	return 0;
}
