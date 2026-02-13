// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/slub.h>

static struct kmem_global_fls global_fls = {0};

inline void *kmem_get_parent_page(struct kmem_obj *obj)
{
	return obj->parent_cache->parent_page;
}

static inline struct kmem_fl *kmem_get_curr_fl(void)
{
	int core_id = curr_core_id();
	
	return &global_fls.cache[core_id];
}

static inline struct kmem_cache *kmem_get_cache(struct kmem_fl *fl, int order)
{
	return fl->cache[order];
}

static inline void kmem_set_cache(struct kmem_cache *c, struct kmem_fl *fl, 
				  int order)
{
	fl->cache[order] = c;
}

static inline struct kmem_cache *kmem_get_curr_cache(int order)
{
	struct kmem_fl *fl = kmem_get_curr_fl();

	return fl->cache[order];
}

static void kmem_set_curr_cache(struct kmem_cache *c, int order)
{
	struct kmem_cache *fl_c = kmem_get_curr_cache(order);
	struct kmem_fl *fl; 
	
	// first entry
	if (!fl_c) {
		fl = kmem_get_curr_fl();
		fl->cache[order] = c;
		return;
	}

	// go to end
	while (fl_c->next)
		fl_c = fl_c->next;

	fl_c->next = c;
}


/* for a given base address and order, create a new object 
 * for the current core.
 *
 * Also responsible for keeping track of the metadata tail
 *
 * NOTE: we always create an object only for the native core. This
 * helps keep things simple
 */
static struct kmem_obj *kmem_create_obj (int order, void *base_addr)
{
	return 0;
}

static void kmem_populate_cache(struct kmem_cache *c)
{
	//
}

// NOTE: All create functions are responsible for the tail to be updated
struct kmem_cache *kmem_create_cache(int order, struct kmem_fl *fl)
{
	struct kmem_cache *c = fl->end;

	fl->end = (void *) ((u64)fl->end + sizeof(struct kmem_cache));
	c->order = order;
	c->parent_fl = fl;

	c->parent_page = page_alloc(1);
	c->empty = KMEM_CACHE_AVAIL;
	c->next = 0;
	
	kmem_populate_cache(c);

	return c;
}

static void kmem_init_cache()
{
	/*
	 * initialize a cache for all order pages.
	 *
	 * All cache metadata must be stored in a page allocated using the 
	 * page allocator. This allows future objects to be 
	 * distributed/allocated/freed much more easily 
	 */
}

void *slub_alloc(size_t size)

{
	/* Keep a list of smaller occupied blocks
	 * which can be freed. This can also contain 
	 * metadata about the bigger block/chunk its being
	 * allocated from. Essentially it needs to be a smaller
	 * page allocator - SLUB allocation.
	 *
	 * Each slab must contain objects of a fixed len. This way,
	 * allocations can be fast and different pages can serve different
	 * lengths
	 */

	struct kmem_fl *fl = kmem_get_curr_fl();

	if (!fl)
		kmem_init_cache();

	return 0;
}
