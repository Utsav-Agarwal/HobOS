// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/compiler_types.h>
#include <hobos/slub.h>
#include <hobos/task.h>

static struct kmem_global_fls global_fls = {0};

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

	return fl->cache[ORDER_OFFSET(order)];
}

void kmem_print_obj(struct kmem_obj *obj)
{
	kprintf("order: %x\n", obj->order);
	kprintf("addr: %x\n", obj->addr);
	kprintf("parent_core_id: %x\n", obj->parent_core_id);
	kprintf("next: %x\n", obj->next);
}

void kmem_print_cache(struct kmem_cache *c)
{
	struct kmem_obj *obj = c->first;

	kprintf("%x\n", c);
	kprintf("fl: %x\n", c->parent_fl);
	kprintf("order: %x\n", c->order);
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

void kmem_print_fl(struct kmem_fl *fl)
{
	int i;

	kprintf("core_id: %x\n", fl->core_id);
	kprintf("end: %x\n", fl->end);

	for (i = 0; i < MAX_NR_CACHE; i++)
		kmem_print_cache(fl->cache[i]);
}

static void kmem_add_cache(struct kmem_cache *c)
{
	int order = c->order;
	struct kmem_cache *fl_c = kmem_get_curr_cache(order);
	struct kmem_fl *fl;
	int cache_index = ORDER_OFFSET(order);

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

static void kmem_add_obj(struct kmem_cache *c, struct kmem_obj *obj)
{
	struct kmem_obj *n_obj;

	// new slab
	if (!c->first) {
		c->first = obj;
		c->parent_page = (void *)((u64)obj->addr & ~0xFFF);
		obj->next = 0;
		return;
	}

	n_obj = c->first;
	while (n_obj->next)
		n_obj = n_obj->next;

	n_obj->next = obj;
	if (!kmem_cache_is_available(c))
		c->status = KMEM_CACHE_AVAIL;
}

/* for a given base address and order, create a new object
 * for the current core.
 *
 * Also responsible for keeping track of the metadata tail
 *
 * NOTE: we always create an object only for the native core. This
 * helps keep things simple
 */
static struct kmem_obj *kmem_create_obj(struct kmem_cache *c)
{
	struct kmem_obj *n_obj;
	struct kmem_obj *obj;
	int order = c->order;
	size_t blk_offset = KMEM_OBJECT_SIZE(order);
	struct kmem_fl *fl = c->parent_fl;

	if (!c)
		return 0;

	obj = (struct kmem_obj *)fl->end;
	fl->end = (void *)((u64)fl->end + sizeof(struct kmem_obj));
	obj->order = order;
	obj->parent_core_id = curr_core_id();
	obj->parent_cache = c;
	obj->next = 0;

	// new slab
	if (!c->first) {
		obj->addr = page_alloc(1);
	} else {
		n_obj = c->first;
		while (n_obj->next)
			n_obj = n_obj->next;

		obj->addr = (void *)
			((u64)n_obj->addr + blk_offset);
	}

	kmem_add_obj(c, obj);
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
static struct kmem_cache *kmem_create_cache(int order, u64 flags)
{
	struct kmem_fl *fl = kmem_get_curr_fl();
	struct kmem_cache *c;

	if (!fl->end)
		fl->end = page_alloc(1);

	// page alloc failed
	if (!fl->end)
		return 0;

	c = fl->end;
	//TODO: maybe put a mutex here since its a global struct?
	fl->end = (void *)((u64)fl->end + sizeof(*c));
	c->order = order;
	c->parent_fl = fl;

	c->status = KMEM_CACHE_N_AVAIL;
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
	int i = 0;

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

static inline struct kmem_obj *kmem_extract_obj_from_cache(struct kmem_cache *c)
{
	struct kmem_obj *obj = c->first;
	struct kmem_obj *p_obj = obj;

	if (!obj)
		return 0;

	while (obj->next) {
		p_obj = obj;
		obj = obj->next;
	}

	p_obj->next = 0;
	return obj;
}

static inline struct kmem_obj *kmem_extract_obj_from_fl(int order)
{
	struct kmem_cache *c = kmem_get_curr_cache(order);

	return kmem_extract_obj_from_cache(c);
}

static void kmem_add_to_task(struct task *t, struct kmem_obj *obj)
{
	struct ctxt *ctxt = t->ctxt;
	struct kmem_obj *o = ctxt->used_kmem;

	obj->next = 0;
	if (!o) {
		ctxt->used_kmem = obj;
		return;
	}

	while (o->next)
		o = o->next;

	o->next = obj;
}

static inline struct kmem_obj *kmem_acquire_obj(size_t size)
{
	int order = kmem_get_obj_order(size);

	return kmem_extract_obj_from_fl(order);
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
	struct kmem_obj *obj;
	struct task *t;

	// init as an when needed
	if (!fl->cache[0])
		kmem_init_caches();

	obj = kmem_acquire_obj(size);
	if (!obj)
		return 0;

	t = get_curr_task();
	if (!t)
		return 0;

	kmem_add_to_task(t, obj);
	return obj->addr;
}

/*
 * Return the parent object which points to this addr. If this is a single entry,
 * return it.
 */
static inline struct kmem_obj *kmem_get_obj_parent(struct kmem_obj *objs,
						   void *addr)
{
	struct kmem_obj *o = objs;
	struct kmem_obj *p = o;

	if ((!addr) || (!objs))
		return 0;

	//maybe its the first element
	if (o->addr == addr)
		return o;

	while (o) {
		if (o->addr == addr)
			return p;

		p = o;
		o = o->next;
	}

	return 0;
}

/*
 * Return parent if no child.
 * Return 0 if null.
 */
static inline struct kmem_obj *kmem_extract_obj_child(struct kmem_obj *parent)
{
	struct kmem_obj *child;

	if (!parent)
		return parent;

	if (!parent->next)
		return parent;

	child = parent->next;
	parent->next = child->next;
	child->next = 0;
	return child;
}

static struct kmem_obj *kmem_extract_obj(struct task *t, void *addr)
{
	struct ctxt *ctxt = t->ctxt;
	struct kmem_obj *objs = ctxt->used_kmem;
	struct kmem_obj *target_obj;

	target_obj = kmem_get_obj_parent(objs, addr);
	target_obj = kmem_extract_obj_child(target_obj);
	return target_obj;
}

static struct kmem_cache *kmem_find_relative_cache(struct kmem_obj *obj)
{
	void *parent_page = (void *)((u64)obj->addr & ~0xFFF);
	struct kmem_cache *c;
	int order = obj->order;

	c = kmem_get_curr_cache(order);
	while (c) {
		if (c->parent_page == parent_page)
			return c;

		c = c->next;
	}

	return 0;
}

static void kmem_reclaim_obj(struct kmem_obj *obj)
{
	int order = obj->order;
	struct kmem_cache *c;
	int core_id;

	core_id = curr_core_id();

	/*
	 * We are home
	 */
	if (core_id == obj->parent_core_id) {
		c = kmem_get_curr_cache(order);
		kmem_add_obj(c, obj);
		return;
	}

	/*
	 * If not, check if a previous cache exists from
	 * non-natively freed pages. If so, just add it there.
	 *
	 * Else, create a new empty cache and add it there.
	 *
	 * TODO: These stray caches can probably be flushed out/trimmed
	 * when/if caches are synced between cores?
	 */
	c = kmem_find_relative_cache(obj);
	if (!c)
		c = kmem_create_cache(order, KMEM_CACHE_CREATE_ONLY);

	kmem_add_obj(c, obj);
}

int slub_free(void *addr)
{
	/*
	 * If kfree() addr belongs to thread's kmem_objs, then it was allocated
	 * with slub, else forward to page_free() by returning.
	 */

	struct kmem_obj *o;
	struct task *t;

	if (!addr)
		return 0;

	t = get_curr_task();
	o = kmem_extract_obj(t, addr);

	/*
	 * Nothing to free here
	 */
	if (!o)
		return -1;

	/*
	 * Object reclaimed
	 */
	kmem_reclaim_obj(o);
	return 0;
}
