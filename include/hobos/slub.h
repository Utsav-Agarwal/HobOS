/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SLUB_H
#define SLUB_H

#include <hobos/kstdio.h>
#include <hobos/mmu.h>
#include <hobos/page_alloc.h>
#include <hobos/smp.h>
#include <hobos/types.h>

/* 2^order B objects */
#define MIN_ORDER_KMEM			6
#define MAX_ORDER_KMEM			10
#define KMEM_OBJECT_SIZE(order)		(1 << order)

#define KMEM_CACHE_CREATE_ONLY		0xAB

#define KMEM_CACHE_AVAIL		0x0
#define KMEM_CACHE_N_AVAIL		0x1

#define MAX_CPU_NR_CACHE		(MAX_REMOTE_CORE_ID + 1)

/*
 * We want to localise the operations as much as possible. 
 * This allows lockless ops since it is not visible to
 * other cores.
 */

struct kmem_obj {
	void *addr;
	int order;
	struct kmem_cache *parent_cache;
	unsigned int parent_core_id;
	struct kmem_obj *next;
};

struct kmem_cache {
	int order;
	struct kmem_fl *parent_fl;
	void *parent_page;
	struct kmem_obj *first;
	bool status;
	struct kmem_cache *next; /* if we have more than 1 parent page, 
				    lets say if coming 
				    from another processor or init is full*/
};

struct kmem_fl {
	struct kmem_cache *cache[MAX_ORDER_KMEM - MIN_ORDER_KMEM + 1];
	volatile void *end;	// data is stored sequentially, 
				// this marks the tail
	int core_id;
};

struct kmem_global_fls {
	struct kmem_fl fls[MAX_CPU_NR_CACHE];
};

void kmem_fl_sy (void);				/* sync free lists and 
						   consolidate page 
						   blocks to resolve 
						   fragmentation */
void *slub_alloc(size_t size);			/* allocate small object */

#endif
