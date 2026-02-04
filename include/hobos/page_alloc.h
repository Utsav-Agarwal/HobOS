/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#include <hobos/types.h>

#define MAX_PAGE_ORDER	11

// 2^order = number of pages in block
#define PAGE_BLOCK(order)	(1 << (order))
#define MAX_PAGE_BLOCK	PAGE_BLOCK(MAX_PAGE_ORDER)

#define PAGE_SIZE	4096UL

#define BLOCK_USED	0xDEDE

struct page_block {
	void *page;
	int order;
	unsigned int flags;
	struct page_block *next;
};

// a list per order
struct free_list {
	struct page_block *first;
	unsigned int count;
};

struct free_list_header {
	volatile struct free_list list[MAX_PAGE_ORDER + 1];
	volatile struct page_block *last_entry;
};

extern volatile unsigned char __phymem_end;

void *page_alloc(unsigned int nr_pages);
void page_free(void *pfn);
void init_free_list(u64 addr, size_t size);
void print_free_lists(void);

#endif
