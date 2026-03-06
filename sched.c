// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/types.h>
#include <hobos/sched.h>

void save_ctxt(struct task *task)
{
	struct ctxt *ctxt = task->ctxt;

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

void sched_save_ctxt(struct task *t)
{
	save_ctxt(t);
	//TODO: sched_idle - we want states to be consistent from when we
	//resume
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

__unused __noreturn static void sched_switch(struct task *prev, struct task *next)
{
	sched_save_ctxt(prev);
	sched_run(next);
}

/* queue next */
void schedule(void)
{
	//TODO
}
