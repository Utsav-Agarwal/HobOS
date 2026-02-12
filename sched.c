// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/types.h>
#include <hobos/sched.h>

mutex_t sched_mutex = 0;

inline void save_ctxt(struct task *task)
{
	struct ctxt *ctxt = &task->ctxt;

	/*
	 * Reserve a new stack frame. Once all is ready, we can go ahead
	 * and load the context structure with the stack frame.
	 *
	 * We cannot store the context straightaway since it will require
	 * a general purpose register to store the loadaddr.
	 */
	asm volatile ("mov x0, %0"
    		      "stp x10, x11, [x0, #16*0]"
    		      "stp x12, x13, [x0, #16*1]"
    		      "stp x14, x15, [x0, #16*2]"
    		      "stp x16, x17, [x0, #16*3]"
    		      "stp x18, x19, [x0, #16*4]"
    		      "stp x20, x21, [x0, #16*5]"
    		      "stp x22, x23, [x0, #16*6]"
    		      "stp x24, x25, [x0, #16*7]"
    		      "stp x26, x27, [x0, #16*8]"
    		      "stp x28, x29, [x0, #16*9]"
    		      "str x30, [x0, #16*10]"
		      ::"r"(ctxt));
}

inline void resume_ctxt(struct task *task)
{
	struct ctxt *ctxt = &task->ctxt;
	
	asm volatile ("mov x0, %0"
		      "ldp x10, x11, [x0, #16*0]"
		      "ldp x12, x13, [x0, #16*1]"
		      "ldp x14, x15, [x0, #16*2]"
		      "ldp x16, x17, [x0, #16*3]"
		      "ldp x18, x19, [x0, #16*4]"
		      "ldp x20, x21, [x0, #16*5]"
		      "ldp x22, x23, [x0, #16*6]"
		      "ldp x24, x25, [x0, #16*7]"
		      "ldp x26, x27, [x0, #16*8]"
		      "ldp x28, x29, [x0, #16*9]"
		      "ldr x30, [x0, #16*10]"
		      ::"r"(ctxt));
}

