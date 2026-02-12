/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SYSCALL_NR_H
#define SYSCALL_NR_H

#define SUPP_SYSCALL_NR	(4UL)	/* Total syscalls supported */

#define SYSCALL_NI	(0UL)	/* Not implemented */
#define SYSCALL_PUTC	(1UL)	/* putc */
#define SYSCALL_PUTS	(2UL)	/* puts */
#define SYSCALL_PANIC	(3UL)	/* Non-recoverable system panic */

#endif
