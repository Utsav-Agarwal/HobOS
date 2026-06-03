/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <hobos/task.h>

struct workqueue {
	int core_id;		//mostly needed for debugging
	struct task *queue;
};

struct workqueue *wq_get_curr(void);
bool wq_is_empty(struct workqueue *wq);
void wq_init(void);

struct task *wq_queue_next(struct workqueue *wq);
void wq_push(struct workqueue *wq, struct task *t);
#endif
