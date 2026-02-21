// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/types.h>
#include <hobos/sched.h>

mutex_t sched_mutex;

inline void save_ctxt(struct task *task)
{
	struct ctxt *ctxt = task->ctxt;

	asm volatile ("mov x0, %0\n"
		      "stp x10, x11, [x0, #16*0]\n"
		      "stp x12, x13, [x0, #16*1]\n"
		      "stp x14, x15, [x0, #16*2]\n"
		      "stp x16, x17, [x0, #16*3]\n"
		      "stp x18, x19, [x0, #16*4]\n"
		      "stp x20, x21, [x0, #16*5]\n"
		      "stp x22, x23, [x0, #16*6]\n"
		      "stp x24, x25, [x0, #16*7]\n"
		      "stp x26, x27, [x0, #16*8]\n"
		      "stp x28, x29, [x0, #16*9]\n"
		      "str x30, [x0, #16*10]"
		      ::"r"(ctxt));
}

inline void resume_ctxt(struct task *task)
{
	struct ctxt *ctxt = task->ctxt;

	asm volatile ("mov x0, %0\n"
		      "ldp x10, x11, [x0, #16*0]\n"
		      "ldp x12, x13, [x0, #16*1]\n"
		      "ldp x14, x15, [x0, #16*2]\n"
		      "ldp x16, x17, [x0, #16*3]\n"
		      "ldp x18, x19, [x0, #16*4]\n"
		      "ldp x20, x21, [x0, #16*5]\n"
		      "ldp x22, x23, [x0, #16*6]\n"
		      "ldp x24, x25, [x0, #16*7]\n"
		      "ldp x26, x27, [x0, #16*8]\n"
		      "ldp x28, x29, [x0, #16*9]\n"
		      "ldr x30, [x0, #16*10]"
		      ::"r"(ctxt));
}

