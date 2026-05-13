// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/types.h>
#include <hobos/sched.h>
#include <hobos/timer.h>
#include <hobos/workqueue.h>

//#define DEBUG

extern struct task idle_task;

__noreturn static void fail(char *msg)
{
	kprintf("PANIC: %s\n", msg);
	while (1)
		;
}

static void sched_switch(struct task *prev, struct task *next)
{
	volatile struct ctxt *ctxt;

#ifdef DEBUG
	if (prev)
		kprintf("SWITCH: [%x] -> [%x]\n", prev->pid, next->pid);
	else
		kprintf("RUN: [%x]\n", next->pid);
#endif
	
	ctxt = prev->ctxt;
	asm volatile ("mov %0, sp\n" : "=r"(ctxt->sp));
	asm volatile ("mrs %0, spsr_el1\n" : "=r"(ctxt->spsr));
	asm volatile ("mov x0, %0\n"
		      "stp x19, x20, [x0, #16*0]\n"
		      "stp x21, x22, [x0, #16*1]\n"
		      "stp x23, x24, [x0, #16*2]\n"
		      "stp x25, x26, [x0, #16*3]\n"
		      "stp x27, x28, [x0, #16*4]\n"
		      "stp x29, x30, [x0, #16*5]\n"
		      :
		      : "r"(ctxt->sp)
		      : "x0", "memory");

	ctxt->sp -= 12*6;	//reserve stack
	/* Ensure that all previous memory operations have been completed
	 * since ctxt read/wrties are load/stores
	 */

	/* Is this what we want to resume? */
	if (prev->resume == 1) {
		prev->resume = 0; 
		return;
	}

	
	/* Resume this next time */
	next->resume = 1; 
	if (kthread_start(next) < 0)
		fail("got NULL thread!\n");

	ctxt = next->ctxt;
	asm volatile ("sub sp, sp, #16*6\n"
		      "ldp x19, x20, [sp, #16*0]\n"
		      "ldp x21, x22, [sp, #16*1]\n"
		      "ldp x23, x24, [sp, #16*2]\n"
		      "ldp x25, x26, [sp, #16*3]\n"
		      "ldp x27, x28, [sp, #16*4]\n"
		      "ldp x29, x30, [sp, #16*5]\n"
		      "mov %0, x0\n"
		      : "=r"(ctxt->sp)
		      :
		      : "x0", "memory");

	asm volatile ("msr spsr_el1, %0\n":: "r"(ctxt->spsr));
	asm volatile ("mov sp, %0\n"
		      "add sp, sp, #16*6\n"
		      "dmb ish\n"
		      "ret\n"
		      :: "r"(ctxt->sp));

	wmb();
}

/*
 * Each processor will have its own workqueue. This means, each processor
 * can manage its own workqueue - i.e, let the scheduler logic be local 
 * and the only thing we need to do when a new thread comes in is queue it
 * in the respective workqueue (depending on scheduling strategy - such as simple
 * load balancing).
 */

/* queue next */
/* we follow Round Robin for now */
void yield(void)
{
	struct workqueue *wq = wq_get_curr();
	struct task *next, *t = get_curr_task();


	// nothing to do
	if (!wq) {
		fail("workqueue not initialized!\n");
		return;
	}
	
	if (wq_is_empty(wq) || !wq->queue->next)
		return;

	next = wq_queue_next(wq);
	if (!t)
		t = &idle_task;

	if (!next)
		next = t;

	sched_switch(t, next);
}

int volatile schedule_needed;
u64 __handle_sched_irq(u64 sp)
{
	struct workqueue *wq = wq_get_curr();
	struct task *next, *t = get_curr_task();

	t->ctxt->sp = (void *)sp;

	global_timer.reset_timer(&global_timer);
	global_timer.set_timer(&global_timer, 0x1000);
	
	/* if nothing else, sit idle */
	next = wq_queue_next(wq);
	if (kthread_start(next) < 0) {
		kthread_queue(&idle_task);
		next = wq_queue_next(wq);
	}

#ifdef DEBUG
      kprintf("preempted! (%x)->(%x)\n", t->pid, next->pid);
#endif
	
	return (u64)next->ctxt->sp;
}
