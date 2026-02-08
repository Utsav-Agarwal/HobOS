// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/page_alloc.h>
#include <hobos/mmu.h>
#include <hobos/compiler_types.h>
#include <hobos/kstdio.h>

/*
 * We want a buddy allocator.
 *
 * Essentially, we divide the memory into regions in
 * the order of 2^n pages.
 *
 * This means, all memory assigned will always be contiguous.
 * Any memory requested which is bigger than the largest order
 * will not be serviced.
 *
 * NOTE: Page length is always fixed to 4096 B.
 * NOTE: Max memory order is 8 MB = 2^11 -> 11 to avoid fragmentation
 * and management complexity
 */

/*
 * Free list contains all page blocks. Each block has a used flag. If this
 * is enabled, the block cannot be merged with its buddy. Once it is freed,
 * this flag is cleared.
 *
 * We maintain a list of page blocks which are acquired to decrease search time.
 */
static struct free_list_header fl_meta __section(".phymem_meta");

static inline struct free_list *get_block_free_list(int order)
{
	return &fl_meta.list[order];
}

static inline size_t get_list_size(int order)
{
	struct free_list *list = get_block_free_list(order);

	return (PAGE_BLOCK(order) * list->count * PAGE_SIZE);
}

static inline u64 block_pfn(volatile struct page_block *pb)
{
	return (u64)(pb->page);
}

static inline u64 get_buddy_pfn(volatile struct page_block *pb)
{
	return (PAGE_SIZE << pb->order) ^ block_pfn(pb);
}

static inline size_t block_phy_offset(int order)
{
	return get_list_size(order);
}

static inline bool block_is_used(volatile struct page_block *pb)
{
	return !!(pb->flags & BLOCK_USED);
}

static inline bool block_is_free(volatile struct page_block *pb)
{
	return !block_is_used(pb);
}

static inline bool page_block_exists(int order)
{
	if (get_block_free_list(order)->count == 0)
		return 0;

	return 1;
}

static bool list_has_free_block(int order)
{
	struct free_list *list = get_block_free_list(order);
	volatile struct page_block *pb = list->first;
	int i = 0;

	if (!page_block_exists(order))
		return 0;

	while ((i < list->count)) {
		if (!pb) {
			kprintf("WARNING: count and actual list not aligned!\n");
			kprintf("count: [%d], elements: [%d]\n", list->count, i);
			return 0;
		}

		if (block_is_free(pb))
			return 1;

		pb = pb->next;
		i++;
	}

	return 0;
}

static void print_page_block(volatile struct page_block *pb)
{
	kprintf("|addr:\t|0x%x|", (u64)pb->page);
	if (block_is_free(pb))
		kprintf("[free]");
	else
		kprintf("[used]");

	kprintf("\n");
}

static void print_block_list(struct free_list *list)
{
	volatile struct page_block *pb = list->first;
	volatile int curr_order = 0;
	size_t blk_sz_kb = 0;
	int i = 0;

	if (!pb)
		return;

	curr_order = pb->order;
	blk_sz_kb = get_list_size(curr_order);
	blk_sz_kb = blk_sz_kb / 1024;
	kprintf("O[%d][%x pages]:\t%d blocks\t(%d KiB)\n",
		curr_order,
		PAGE_BLOCK(curr_order),
		fl_meta.list[curr_order].count,
		blk_sz_kb);

	for (i = 0; i < list->count; i++) {
		print_page_block(pb);
		pb = pb->next;
	}
}

void print_free_lists(void)
{
	int i = 0;

	for (i = MAX_PAGE_ORDER; i >= 0; i--) {
		print_block_list(get_block_free_list(i));
	}
}

static volatile struct page_block *new_page_block(void *page_addr)
{
	volatile struct page_block *pb = fl_meta.last_entry;

	if (pb)
		pb++;
	else
		pb = (struct page_block *)
			((char *)&fl_meta + sizeof(struct free_list_header));

	pb->page = page_addr;
	pb->next = 0;
	pb->flags = 0;

	fl_meta.last_entry = pb;
	return pb;
}

static void add_block_to_order(volatile struct page_block *pb, int order)
{
	struct free_list *order_list = get_block_free_list(order);
	volatile struct page_block *cb = 0;

	pb->order = order;
	if (!order_list->first)
		order_list->first = pb;
	else {
		cb = order_list->first;
		while (cb->next)
			cb = cb->next;

		cb->next = pb;
	}

	order_list->count++;
}

static inline volatile struct page_block *get_nth_block(int order, int index)
{
	struct free_list *list = get_block_free_list(order);
	volatile struct page_block *pb = list->first;
	int i = index;

	if (index >= list->count)
		return 0;

	while (i > 0) {
		pb = pb->next;
		i--;
	}

	return pb;
}

static inline void delete_block(volatile struct page_block *pb)
{
	memset((void *)pb, 0, sizeof(struct page_block));
}

static int remove_page_block(volatile struct page_block *pb)
{
	struct free_list *list = get_block_free_list(pb->order);
	volatile struct page_block *cb = list->first;

	if (list->count == 1) {
		list->count--;
		list->first = 0;
		return 0;
	}

	while (cb) {
		if (cb->next == pb) {
			list->count--;
			cb->next = pb->next;
			pb->next = 0;
			return 0;
		}

		cb = cb->next;
	}

	return 1;
}

/*
 * We can simply ignore removing alot of meta data and just decrement
 * the counter
 */
static int remove_next_free_block(int order)
{
	struct free_list *list = get_block_free_list(order);
	volatile struct page_block *pb = 0;
	int i = 0;

	if (list->count <= 0)
		return 1;

	if (list->count == 1) {
		if (block_is_used(list->first)) {
			return 1;
		} else {
			list->count--;
			list->first = 0;
			return 0;
		}
	}

	for (i = list->count - 1; i > 0; i--) {
		pb = get_nth_block(list->first->order, i - 1);

		if (block_is_free(pb->next)) {
			//kprintf("removed %x\n", pb->page);
			pb->next = 0;
			list->count--;
			return 0;
		}
	}

	return 1;
}

/*
 * Returns the last added block for order (easiest to split)
 */
static volatile struct page_block *get_next_free_block(int order)
{
	struct free_list *list = get_block_free_list(order);
	volatile struct page_block *pb = fl_meta.last_entry;
	int i = list->count;

	if ((pb->order == order) && block_is_free(pb))
		return pb;

	while (i > 0) {
		pb = get_nth_block(order, i - 1);
		if (!pb)
			return 0;

		if (block_is_free(pb))
			return pb;

		i--;
	}

	return 0;
}

/*
 * Split the next free block in a given order
 */
static int split_page_block(int order)
{
	volatile struct page_block *pb = get_next_free_block(order);
	u64 base_addr = 0;

	__unused u64 page_offset = 0;
	int curr_order = order;

	if (pb)
		base_addr = (u64)pb->page;

	if (remove_next_free_block(curr_order))
		return 1;

	//kprintf("%x = ", pb->page);
	// Next order buddy creation
	page_offset = PAGE_BLOCK(order - 1) * PAGE_SIZE;

	pb = new_page_block((void *)base_addr);
	add_block_to_order(pb, order - 1);
	//kprintf("%x + ", pb->page);

	barrier();
	pb = new_page_block((void *)get_buddy_pfn(pb));
	add_block_to_order(pb, order - 1);
	//kprintf("%x (%x)\n", pb->page, page_offset);
	return 0;
}

/*
 * Remove from current order
 * Create a pair of buddies with lower order block size
 * Add the pair of buddies to lower order
 */
static volatile struct page_block *reduce_to_order(int order, int target_order)
{
	volatile int curr_order = order;

	// Keep splitting the last block until target + 1 order block is split
	while (curr_order > target_order) {
		dmb(ish);
		if (split_page_block(curr_order))
			return 0;

		curr_order--;
	}

	return get_next_free_block(target_order);
}

static void create_free_list(u64 start_addr, int order, size_t size)
{
	volatile struct page_block *cb = fl_meta.last_entry;
	u64 curr_addr = start_addr;
	size_t nr_blocks = size;

	if (!nr_blocks)
		return;

	while (nr_blocks--) {
		if (!cb) {
			cb = new_page_block((void *)curr_addr);
		} else {
			cb->next = new_page_block((void *)curr_addr);
			cb = cb->next;
		}

		add_block_to_order(cb, order);
		curr_addr += block_phy_offset(order);
	}
}

__unused static void simple_page_alloc_test(void)
{
	volatile int i = 0;
	volatile int j = 0;
	void *x = 0;

	kprintf("\n\n** Running simple page alloc test **\n\n");
	kprintf("\nStarting page block map:\n");
	print_free_lists();

	while (i < 8) {
		// Test both buddies
		while (j < 2) {
			x = page_alloc(PAGE_BLOCK(i));
			if (x == 0) {
				kprintf("Test failed at alloc order: %d\n", i);
				return;
			}

			page_free(x);
			j++;
		}

		i++;
		j = 0;
	};

	kprintf("\nFinal page block map:\n");
	print_free_lists();

	kprintf("\n\n** Test finished **\n\n");
}

/*
 * This is called at the very beginning and is responsible for initializing
 * the free_lists for all orders.
 */
void init_free_list(u64 addr, size_t size)
{
	unsigned int total_pages = size / PAGE_SIZE;
	int curr_order = MAX_PAGE_ORDER - 1;
	size_t curr_order_pages = 0;

	fl_meta.last_entry = 0;

	kprintf("Starting memory management from addr 0x%x\n", addr - size);
	// We want to distribute per order
	while (curr_order >= 0) {
		curr_order_pages = total_pages / PAGE_BLOCK(curr_order);
		create_free_list(addr, curr_order, curr_order_pages);

		if (curr_order_pages) {
			total_pages = total_pages % PAGE_BLOCK(curr_order);
		}

		curr_order--;
	}

	print_free_lists();
}

static int get_page_order(size_t nr_pages)
{
	size_t val = nr_pages;
	int i = 0;

	while (val < PAGE_BLOCK(i))
		i++;

	return i;
}

/*
 * Return last free block. If none, split and create
 */
static volatile struct page_block *get_block(int order)
{
	volatile int curr_order = order;

	// find the closest order to get block from
	while (curr_order < MAX_PAGE_ORDER) {
		if (list_has_free_block(curr_order))
			break;

		curr_order++;
	}

	// No contiguous memory block can satisfy this
	if (curr_order > MAX_PAGE_ORDER)
		return 0;

	return reduce_to_order(curr_order, order);
}

/*
 * Mark block used
 */
static inline void mark_block_used(volatile struct page_block *pb)
{
	barrier();
	pb->flags = BLOCK_USED;
}

/*
 * Mark block free
 */
static inline void mark_block_free(volatile struct page_block *pb)
{
	barrier();
	pb->flags = 0;
}

/*
 * Returns NULL or 0 if fails
 */
void *page_alloc(unsigned int nr_pages)
{
	volatile struct page_block *pb = 0;
	int order = 0;

	if (!nr_pages)
		return 0;

	barrier();
	if (nr_pages > MAX_PAGE_BLOCK)
		return 0;

	order = get_page_order(nr_pages);
	pb = get_block(order);
	if (!pb)
		return 0;

	mark_block_used(pb);
	return pb->page;
}

static volatile struct page_block *find_pfn_in_order(void *pfn, int order)
{
	struct free_list *list = get_block_free_list(order);
	volatile struct page_block *pb = list->first;

	while (pb) {
		if (pb->page == pfn)
			return pb;

		pb = pb->next;
	}

	return 0;
}

/*
 * Start looking from the lowest order since programs
 * would usually not request large memory sizes.
 */
static volatile struct page_block *find_block_by_pfn(void *pfn)
{
	volatile struct page_block *pb = 0;
	int order = 0;

	for (order = 0; order <= MAX_PAGE_ORDER; order++) {
		pb = find_pfn_in_order(pfn, order);
		if (pb)
			break;
	}

	return pb;
}

static volatile struct page_block *get_block_buddy(volatile struct page_block *pb)
{
	void *buddy_pfn = (void *)get_buddy_pfn(pb);

	return find_pfn_in_order(buddy_pfn, pb->order);
}

static void merge_buddy(volatile struct page_block *pb)
{
	volatile struct page_block *buddy_pb = 0;
	void *final_pfn = pb->page;
	int order = pb->order;

	buddy_pb = get_block_buddy(pb);

	if (!buddy_pb)
		return;

	/*
	 * Seconday block or buddy is always created at a higher
	 * address
	 */
	if (block_pfn(pb) > block_pfn(buddy_pb))
		final_pfn = (void *)block_pfn(buddy_pb);

	if (block_is_used(pb) || block_is_used(buddy_pb))
		return;

	if (remove_page_block(pb))
		return;

	if (remove_page_block(buddy_pb))
		return;

	buddy_pb = new_page_block(final_pfn);
	add_block_to_order(buddy_pb, order + 1);

	// Recursively try and merge as much as possible
	merge_buddy(buddy_pb);
}

/*
 * Given a physical frame number, free the associated block
 * and merge buddies
 */
void page_free(void *pfn)
{
	volatile struct page_block *pb = find_block_by_pfn(pfn);

	if (!pb)
		return;

	mark_block_free(pb);
	merge_buddy(pb);
}
