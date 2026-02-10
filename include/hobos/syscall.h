/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <hobos/types.h>

#define SYSCALL_PUTS	(1UL)

struct syscall_meta {
	u64 fn_args[8];
	u64 syscall_nr;
};

void do_syscall(void);

#endif
