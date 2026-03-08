// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/types.h>
#include <hobos/sched.h>
#include <hobos/workqueue.h>


extern struct task idle_task;

void save_ctxt(struct task *task)
{
	struct ctxt *ctxt;

	if (!task)
		return;

	ctxt = task->ctxt;
	asm volatile ("mov %0, sp\n" : "=r"(ctxt->sp));
	asm volatile ("mov x0, %0\n"
		      "stp x19, x20, [x0, #16*1]\n"
		      "stp x21, x22, [x0, #16*2]\n"
		      "stp x23, x24, [x0, #16*3]\n"
		      "stp x25, x26, [x0, #16*4]\n"
		      "stp x27, x28, [x0, #16*5]\n"
		      "stp x29, x30, [x0, #16*6]\n"
		      : "=r"(ctxt));

	/* Ensure that all previous memory operations have been completed
	 * since ctxt read/wrties are load/stores
	 */
	wmb();
}

void resume_ctxt(struct task *task)
{
	struct ctxt *ctxt = task->ctxt;

	asm volatile ("mov sp, %0\n":: "r"(ctxt->sp));
	asm volatile ("mov x0, %0\n"
		      "ldp x19, x20, [x0, #16*1]\n"
		      "ldp x21, x22, [x0, #16*2]\n"
		      "ldp x23, x24, [x0, #16*3]\n"
		      "ldp x25, x26, [x0, #16*4]\n"
		      "ldp x27, x28, [x0, #16*5]\n"
		      "ldp x29, x30, [x0, #16*6]\n"
		      ::"r"(ctxt));

	/* Ensure that all previous memory operations have been completed
	 * since ctxt read/wrties are load/stores
	 */
	rmb();
}

__noreturn static void fail(char *msg)
{
	kprintf("PANIC: %s\n", msg);
	while (1)
		;
}

__noreturn void sched_run(struct task *t)
{
	set_curr_task(t);

	if (kthread_start(t) < 0)
		fail("got NULL thread!\n");

	if (t->running)
		resume_ctxt(t);

	fail("Got to end of sched run!\n");
}

__noreturn static void sched_switch(struct task *prev, struct task *next)
{
	save_ctxt(prev);
	if (prev)
		kprintf("SWITCH: [%x] -> [%x]\n", prev->pid, next->pid);
	else
		kprintf("RUN: [%x]\n", next->pid);

	sched_run(next);
	fail("Got to end of sched_switch!\n");
}

/*
 * Each processor will have its own workqueue. This means, each processor
 * can manage its own workqueue - i.e, let the scheduler logic be local 
 * and the only thing we need to do when a new thread comes in is queue it
 * in the respective workqueue (depending on scheduling strategy - such as simple
 * load balancing).
 */

static inline void idle(void)
{
	sched_run(&idle_task);
}

static void queue_idle(void)
{
	kthread_queue(&idle_task);
	kprintf("nothing to do...\n");
}

/* queue next */
/* we follow Round Robin for now */
void schedule(void)
{
	struct workqueue *wq = wq_get_curr();
	struct task *next, *t = get_curr_task();

	// nothing to do
	if (!wq) {
		fail("workqueue not initialized!\n");
		return;
	}

	// don't queue next if wq is starting for the first time
	if (t != &idle_task)
		t = wq_queue_next(wq);
	
	// all completed, rest for now
	if (wq_is_empty(wq)) {
		queue_idle();
	}
	
	next = wq->queue;
	sched_switch(t, next);
}
