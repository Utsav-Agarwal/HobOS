// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/workqueue.h>
#include <hobos/lib/stdlib.h>
#include <hobos/smp.h>
#include <hobos/irq.h>

//#define DEBUG

static struct workqueue *global_wqs[MAX_REMOTE_CORE_ID + 1];

static struct workqueue *get_core_workqueue(int core_id)
{
	return global_wqs[core_id];
}

struct workqueue *wq_get_curr(void)
{
	int core_id;

	core_id = curr_core_id();
	return get_core_workqueue(core_id);
}

static void wq_print(struct workqueue *wq)
{
	struct task *t = wq->queue;

	if (!t)
		return;

	while(t) {
#ifdef DEBUG
		kprintf("[%x] < ", t->pid);
#endif
		t = t->next;
	}

#ifdef DEBUG
	kprintf("\n");
#endif
}

/*
 * pop from head 
 */
struct task *wq_pop(struct workqueue *wq)
{
	struct task *t, *next, *_tmp;
	
	t = get_curr_task();

	/* Dont free the task immediately. We need this just until this current
	 * schedule is finished so we can context switch gracefully.
	 *
	 * Cleanup upcoming tasks
	 */
	next = t;
	if (t->next)
		next = t->next;

	while (next) {
		if (has_completed(next)) {
			_tmp = next;
			next = next->next;
			task_free(_tmp);
		} else {
			break;
		}
	}

	/* if next == t, this is the last task */
	if (next == t)
		next = 0;

	wq->queue = next;
	/* Mark end of queue upcoming element
	 * If this was the last task, it could have been freed
	 */
	if (t)
		t->next = 0;

	return t;
}

static inline struct task *get_workqueue_end(struct workqueue *wq)
{
	struct task *t = wq->queue;

	if (!t)
		return 0;

	while (t->next)
		t = t->next;

	return t;
}

void wq_push(struct workqueue *wq, struct task *t)
{
	struct task *tail_t;

	tail_t = get_workqueue_end(wq);
	if (!tail_t) {
		wq->queue = t;
		wq->queue->next = 0;
		return;
	}

	tail_t->next = t;
}

/*
 * Each workqueue is a cyclic queue. The next task to be executed
 * is always going to be the one at the front, i.e, index 0. So to
 * go to next, simply pop and push the first task.
 */
struct task *wq_queue_next(struct workqueue *wq)
{
	struct task *t;
	u64 flags;
	
	flags = irq_save();
	t = wq_pop(wq);
	//do we need to requeue this?
	if (t) {
#ifdef DEBUG
		kprintf("pushing unfinished queue back (%x)\n", t->pid);
#endif
		wq_push(wq, t);
	} 

#ifdef DEBUG
	else {
		kprintf("NULL t!\n");
	}
#endif
	t = wq->queue;
	irq_restore(flags);
	wq_print(wq);
	return t;
}

static inline bool wq_is_not_empty(struct workqueue *wq)
{
	return !!(wq->queue);
}

bool wq_is_empty(struct workqueue *wq)
{
	return !wq_is_not_empty(wq);
}

void wq_init(void)
{
	size_t sz = sizeof(struct workqueue);
	int i;

	//since its not in t	e .bss section, we need to 0 it explicitly
	for (i = 0; i <= MAX_REMOTE_CORE_ID; i++) {
		global_wqs[i] = kmalloc(sz);
		memset(global_wqs[i], 0, sz);
		global_wqs[i]->core_id = i;
	}
}
