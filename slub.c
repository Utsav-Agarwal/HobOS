// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/compiler_types.h>
#include <hobos/slub.h>
#include <hobos/task.h>

static struct kmem_global_fls global_fls = {0};

inline void *kmem_get_parent_page(struct kmem_obj *obj)
{
	return obj->parent_cache->parent_page;
}

static inline struct kmem_fl *kmem_get_curr_fl(void)
{
	int core_id = curr_core_id();
	
	return &global_fls.fls[core_id];
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

	return fl->cache[order - MIN_ORDER_KMEM];
}

void kmem_print_obj(volatile struct kmem_obj *obj)
{
	kprintf("order: %x\n", obj->order);
	kprintf("addr: %x\n", obj->addr);
	kprintf("parent_core_id: %x\n", obj->parent_core_id);
	kprintf("next: %x\n", obj->next);
}

void kmem_print_cache(struct kmem_cache *c)
{
	struct kmem_obj *obj = c->first;

	kprintf("fl: %x\n", c->parent_fl);
	kprintf("order: %x\n", c->order);
	kprintf("page: %x\n", c->parent_page);
	kprintf("first: %x\n", c->first);
	kprintf("next: %x\n", c->next);
	kprintf("status: %x\n", c->status);

	kprintf("\n------\n");
	while (obj) {
		kprintf("%x\n", obj);
		kmem_print_obj(obj);
		obj = obj->next;
		kprintf("\n");
	}
	
	kprintf("===xx===\n\n");
}

void kmem_print_fl (struct kmem_fl *fl)
{
	int i;

	kprintf("core_id: %x\n", fl->core_id);
	kprintf("end: %x\n", fl->end);

	for (i = 0; i < MAX_ORDER_KMEM; i++) {
		kmem_print_cache(fl->cache[i]);
	}
}

static void kmem_add_cache(struct kmem_cache *c)
{
	int order = c->order;
	struct kmem_cache *fl_c = kmem_get_curr_cache(order);
	struct kmem_fl *fl; 
	int cache_index = order - MIN_ORDER_KMEM;

	// first entry
	if (!fl_c) {
		fl = kmem_get_curr_fl();
		fl->cache[cache_index] = c;
		return;
	}

	// go to end
	while (fl_c->next)
		fl_c = fl_c->next;

	fl_c->next = c;
}

static bool kmem_cache_is_available(struct kmem_cache *c)
{
	return !!(c->status == KMEM_CACHE_AVAIL);
}

/* for a given base address and order, create a new object 
 * for the current core.
 *
 * Also responsible for keeping track of the metadata tail
 *
 * NOTE: we always create an object only for the native core. This
 * helps keep things simple
 */
static volatile struct kmem_obj *kmem_create_obj (volatile struct kmem_cache *c)
{
	volatile struct kmem_obj *n_obj;
	volatile struct kmem_obj *obj;
	volatile int order = c->order;
	volatile struct kmem_fl *fl = c->parent_fl;

	if (!c)
		return 0;

	if (!kmem_cache_is_available(c))
		return 0;

	obj = (struct kmem_obj *)fl->end;
	fl->end = (void *)((u64)fl->end + sizeof(struct kmem_obj));
	obj->order = order;
	obj->parent_core_id = curr_core_id();
	obj->parent_cache = c;
	obj->next = 0;

	// new slab
	if (!c->first) {
		c->first = obj;
		obj->addr = page_alloc(1);
		obj->next = 0;
		return obj;
	}

	n_obj = c->first;
	while (n_obj->next)
		n_obj = n_obj->next;

	n_obj->next = obj;
	obj->addr = (void *)((u64)n_obj->addr + KMEM_OBJECT_SIZE(order));
	return obj;
}

static void kmem_populate_cache(struct kmem_cache *c)
{
	int nr_objs = PAGE_SIZE / KMEM_OBJECT_SIZE(c->order);
	struct kmem_obj *obj;
	int i;

	if (!c)
		return;

	for (i = 0; i < nr_objs; i++) {
		obj = kmem_create_obj(c);
		if (!obj)
			kprintf("Error creating kobj!\n");
	}
}

// NOTE: All create functions are responsible for the tail to be updated
// NOTE: creation is always done wrt local caches
static struct kmem_cache *kmem_create_cache(volatile int order, u64 flags)
{
	struct kmem_fl *fl = kmem_get_curr_fl();
	volatile struct kmem_cache *c;

	if (!fl->end)
		fl->end = page_alloc(1);

	// page alloc failed
	if (!fl->end)
		return 0;

	c = fl->end;
	fl->end = (void *) ((u64)fl->end + sizeof(struct kmem_cache));
	c->order = order;
	c->parent_fl = fl;

	c->parent_page = page_alloc(1);
	c->status = KMEM_CACHE_AVAIL;
	c->next = 0;
	
	if (flags != KMEM_CACHE_CREATE_ONLY)
		kmem_populate_cache(c);

	return c;
}

static void kmem_init_caches(void)
{
	/*
	 * initialize a cache for all order pages.
	 *
	 * All cache metadata must be stored in a page allocated using the 
	 * page allocator. This allows future objects to be 
	 * distributed/allocated/freed much more easily 
	 */
	struct kmem_cache *c;
	volatile int i = 0;

	for (i = MIN_ORDER_KMEM; i <= MAX_ORDER_KMEM; i++) {
		c = kmem_create_cache(i, 0);
		kmem_add_cache(c);
	}
}

static inline int kmem_get_obj_order(size_t size)
{
	int i = MIN_ORDER_KMEM;

	while (size > KMEM_OBJECT_SIZE(i)) {
		//kprintf("order: %x for size %x\n", i, size);
		//kprintf("slab size: %x\n", KMEM_OBJECT_SIZE(i));
		i++;
	}

	return i;
}

static inline struct kmem_obj *kmem_cache_obj(struct kmem_cache *c)
{
	struct kmem_obj *obj = c->first;
	struct kmem_obj *p_obj = obj;

	if (!obj)
		return 0;

	while (obj) {
		p_obj = obj;
		obj = obj->next;
	}

	if (obj == c->first)
		c->first = 0;
	else
		p_obj->next = 0;

	return p_obj;
}

static inline struct kmem_obj *kmem_fl_obj(int order)
{
	volatile struct kmem_cache *c = kmem_get_curr_cache(order);

	return kmem_cache_obj(c);
}

static inline struct kmem_obj *kmem_acquire_obj(size_t size)
{
	int order = kmem_get_obj_order(size);

	return kmem_fl_obj(order);
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
	volatile struct kmem_obj *obj;
	volatile struct task *t;

	// init as an when needed
	if (!fl->cache[0])
		kmem_init_caches();

	//kmem_print_fl(fl);
	
	//TODO: call current executing thread and add this to
	//its cache objs so it can be returned properly when freed
	//
	//until processes are formed, we can keep these in a per cpu
	//global list
	obj = kmem_acquire_obj(size);
	if (!obj)
		return 0;

	t = get_curr_task();
	if (!t)
		return 0;

	t->ctxt->used_kmem = obj;
	return obj->addr;
}

