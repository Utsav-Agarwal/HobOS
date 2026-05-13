// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/asm/mutex.h>
#include <hobos/lib/pt_lib.h>
#include <hobos/task.h>
#include <hobos/sched.h>
#include <hobos/workqueue.h>
#include <hobos/irq.h>

mutex_t pid_mutex;
pid_t pid_cntr = 1;

/*
 * Essentially what we want is a 3 task initial structure:
 *
 * PID	Job
 *
 * 0	idle process, run when nothing to do, just sits in a while loop
 * 1	init kernel and then jump to userspace
 * 2	init kernel threading daemon. This is responsible for handling
 *	all kthread_create() calls
 */

int idle(void *data)
{
	while(1)
		asm volatile ("wfe");

	return 0;
}

struct ctxt idle_ctxt;
struct task idle_task = {
		.pid = 0,
		.running = 0,
		.ctxt = &idle_ctxt,
		.pc = idle,
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
		memset(new_task, 0, sizeof(*new_task));
		new_ctxt = kmalloc(sizeof(*new_ctxt));
		memset(new_ctxt, 0, sizeof(*new_ctxt));
		if (!new_task)
			return 0;
	} else {
		return 0;
	}

	memcpy(new_task, task, sizeof(*new_task));
	new_task->pid = assign_new_pid();
	new_ctxt->sp = &new_task->stack[8192];
	new_task->ctxt = new_ctxt;
	return new_task;
}

struct task *get_curr_task(void)
{
	struct workqueue *wq = wq_get_curr();

	if ((pid_cntr == 1) || (!wq))
		return &idle_task;

	return wq->queue;
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
	new_t->resume = 0;
	new_t->completed = 0;
	new_t->data = data;
	wmb();

	return new_t;
}


// we need a barrier here to ensure that older ops dont pass this
static inline void mark_completed(struct task *t)
{
	smp_store_mb(t->resume, 0);
	smp_store_mb(t->completed, 1);
}

/*
 * We need this to exploit the callee saved regs and update the processor
 * state accordingly.
 */
__noreturn void kthread_ret_from_fork(void)
{
	struct task *t;
	int (*thread_fn)(void *data);
	void *data;
	int ret;
	u64 flags;

	flags = irq_save();
	asm volatile("mov %0, x20" : "=r"(data));
	asm volatile("mov %0, x19" : "=r"(thread_fn));
	irq_restore(flags);

	ret = thread_fn(data);

	// finish peacefully
	flags = irq_save();
	t = get_curr_task();
	kprintf("Task (%d) returned with ret: %x\n", ret, t->pid);
	mark_completed(t);
	irq_restore(flags);

	while (1)
		asm volatile ("wfe");
		;
}

void kthread_init_stack(struct task *t)
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
	u64 *x;

	/*
	 * Since we cant use x0-x18 (caller saved regs, can be overwritten)
	 * we need to designate one of the regs to contain the data.
	 */
	ctxt->sp = (void *)((u64)ctxt->sp - 16*6);	// artificially reserve stack
	x = ctxt->sp;
	x[0] = (u64)(t->pc);
	x[1] = (u64)(t->data);
	x[11] = (u64)(kthread_ret_from_fork);

	asm volatile ("msr elr_el1, %0"::"r"(kthread_ret_from_fork));
	
	t->running = 1;

	/* no context to resume */
	t->resume = 0;
	/* Ensure that all previous memory operations have been completed
	 * since ctxt read/wrties are load/stores
	 */
	wmb();
}

void kthread_queue(struct task *t)
{
	struct workqueue *wq = wq_get_curr();

	wq_push(wq, t);
}

void task_free(struct task *t)
{
	//TODO: free all kobjs
	kprintf("freeing [%x]\n", t->pid);
	kfree(t->ctxt);
	kfree(t);
}

bool is_running(struct task *t)
{
	return t->running;
}

int kthread_start(struct task *t)
{
	u64 flags;

	if (!t)
		return -1;
	
	flags = irq_save();
	
	if (!is_running(t)) {
		kprintf("started: %x\n", t->pid);
		kthread_init_stack(t);
	}

	irq_restore(flags);
	return 0;
}

bool has_completed(struct task *t)
{
	return t->completed;
}
