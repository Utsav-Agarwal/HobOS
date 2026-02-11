/* SPDX-License-Identifier: GPL-2.0-only */

.global syscall_table
syscall_table:
	.quad syscall_ni
	.quad syscall_putc
	.quad syscall_puts
	.quad syscall_panic
