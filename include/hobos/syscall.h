/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <hobos/types.h>

struct syscall_meta {
	u64 fn_args[8];
	u64 syscall_nr;
};

typedef void (*syscall_t) (struct syscall_meta *sys_m);

#endif
