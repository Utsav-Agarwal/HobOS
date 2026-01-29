/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#define MAX_PAGE_ORDER	11

// 2^order = number of pages in block
#define PAGE_BLOCK(order)	(1 << ((order) + 1))
#define MAX_PAGE_BLOCK	PAGE_BLOCK(MAX_PAGE_ORDER)

#define PAGE_SIZE	4096UL

struct page_block {
	unsigned int id;
	void *page;
};

// a list per order
struct free_list {
	struct page_block *first;
	unsigned int cnt;
};

void *page_alloc(unsigned int nr_pages);

#endif
