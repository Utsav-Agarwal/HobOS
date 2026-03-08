// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/workqueue.h>
#include <hobos/lib/stdlib.h>
#include <hobos/smp.h>

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

struct task *wq_pop(struct workqueue *wq)
{
	struct task *new_head, *t = wq->queue;

	if (!t)
		return 0;

	new_head = t->next;
	t->next = 0;

	wq->queue = new_head;
	if (has_completed(t)) {
		task_free(t);
		return 0;
	}

	return t;
}

static inline struct task *get_workqueue_end(struct workqueue *wq)
{
	struct task *t = wq->queue;

	if (t)
		while(t->next)
			t = t->next;

	return t;
}

void wq_push(struct workqueue *wq, struct task *t)
{
	struct task *tail_t;

	tail_t = get_workqueue_end(wq);
	if (!tail_t) {
		wq->queue = t;
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

	t = wq_pop(wq);
	
	//do we need to requeue this?
	if (t)
		wq_push(wq, t);

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
