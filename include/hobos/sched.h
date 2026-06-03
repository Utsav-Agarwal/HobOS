/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SCHED_H
#define SCHED_H

#include <hobos/task.h>

void yield(void);
void resume_ctxt(struct task *task);

#endif
