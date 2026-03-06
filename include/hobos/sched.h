/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SCHED_H
#define SCHED_H

#include <hobos/task.h>

void sched_save_ctxt(struct task *t);
void sched_run(struct task *t);
void schedule(void);
void resume_ctxt(struct task *task);

#endif
