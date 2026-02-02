// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/page_alloc.h>
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
 * We need a free list and a used list to track operations when they are freed.
 * This way, free simply has to find the block with the same pfn as the argument
 * and remove/add it back.
 */
static struct free_list_header fl_meta __attribute__((section(".phymem_meta")));
static struct free_list_header ul_meta __attribute__((section(".phymem_meta")));

static inline struct free_list *get_block_free_list(int order)
{
	return &fl_meta.list[order];
}

static void print_page_block(struct page_block *pb)
{
	kprintf("|addr:\t|0x%x|\n", (u64)pb->page, pb->next);
}

static inline size_t get_free_size(int order, unsigned int count)
{
	return (PAGE_BLOCK(order) * count * PAGE_SIZE);
}

static inline size_t block_phy_offset(int order)
{
	return get_free_size(order, 0);
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
	blk_sz_kb = get_free_size(curr_order,
				  fl_meta.list[curr_order].count);

	blk_sz_kb = blk_sz_kb / 1024;
	kprintf("O[%d][%x pages]:\t%d blocks\t(%d KiB)\n",
		curr_order,
		PAGE_BLOCK(curr_order),
		fl_meta.list[curr_order].count,
		blk_sz_kb);

	for (i = 0; i < list->count; i++) {
		print_page_block(pb + i);
	}
}

static void print_free_lists(void)
{
	int i = 0;

	for (i = MAX_PAGE_ORDER; i >= 0; i--) {
		print_block_list(get_block_free_list(i));
	}
}

static struct page_block *new_page_block(void *page_addr)
{
	struct page_block *pb = fl_meta.last_entry;

	if (pb)
		pb++;
	else
		pb = (struct page_block *)
			((char *)&fl_meta + sizeof(struct free_list_header));

	pb->page = page_addr;
	pb->next = 0;

	fl_meta.last_entry = pb;
	return pb;
}

static void add_block_to_order(struct page_block *pb, int order)
{
	struct free_list *order_list = get_block_free_list(order);

	pb->order = order;
	if (!order_list->first)
		order_list->first = pb;

	order_list->count++;
}

/*
 * We can simply ignore removing alot of meta data and just decrement
 * the counter
 */
static void remove_next_free_block(int order)
{
	struct free_list *list = get_block_free_list(order);
	struct page_block *pb = list->first;

	pb += list->count - 1;
	pb->next = 0;
	list->count--;
}

/*
 * Returns the last added block for order (easiest to split)
 */
static struct page_block *get_next_free_block(int order)
{
	struct free_list *list = get_block_free_list(order);
	struct page_block *pb = fl_meta.last_entry;

	// if the last added block was from the same order, just use that
	if (pb->order == order)
		return pb;

	pb = list->first;
	pb += list->count;

	return pb;
}

/*
 * Split the next free block in a given order
 */
static void split_page_block(int order)
{
	struct page_block *pb = get_next_free_block(order);
	u64 page_offset = 0;
	u64 base_addr = (u64)pb->page;
	int curr_order = order;

	remove_next_free_block(curr_order);

	// Next order buddy creation
	page_offset = PAGE_BLOCK(order - 1) * PAGE_SIZE;

	pb = new_page_block((void *)base_addr);
	add_block_to_order(pb, order - 1);

	dmb(ish);
	pb = new_page_block((void *)(base_addr + page_offset));
	add_block_to_order(pb, order - 1);
}

/*
 * Remove from current order
 * Create a pair of buddies with lower order block size
 * Add the pair of buddies to lower order
 */
static struct page_block *reduce_to_order(int order, int target_order)
{
	volatile int curr_order = order;

	// Keep splitting the last block until target + 1 order block is split
	while (curr_order > target_order) {
		split_page_block(curr_order);
		curr_order--;
	}

	return get_next_free_block(target_order);
}

static void create_free_list(u64 start_addr, int order, size_t size)
{
	struct free_list *order_list = get_block_free_list(order);
	struct page_block *cb = fl_meta.last_entry;
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

/*
 * This is called at the very beginning and is responsible for initializing
 * the free_lists for all orders.
 */
void init_free_list(u64 addr, size_t size)
{
	unsigned int total_pages = size / PAGE_SIZE;
	int curr_order = MAX_PAGE_ORDER - 1;
	size_t curr_order_pages = 0;
	size_t blk_sz_kb = 0;

	fl_meta.last_entry = 0;

	kprintf("Starting memory management from addr 0x%x\n", addr);
	kprintf("Available size: %d\n", size);

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

	while (val != PAGE_BLOCK(i)) {
		i++;
	}

	return i;
}

static bool free_page_block_exists(int order)
{
	if (get_block_free_list(order)->count == 0)
		return 0;

	return 1;
}

/*
 * Return last free block. If none, split and create
 */
static struct page_block *get_block(int order)
{
	struct free_list *fl = get_block_free_list(order);
	volatile int curr_order = order;
	struct page_block *pb = 0;

	// find the closest order to get block from
	while (curr_order < MAX_PAGE_ORDER) {
		if (free_page_block_exists(curr_order))
				break;

		curr_order++;
	}

	// No contiguous memory block can satisfy this
	if (curr_order > MAX_PAGE_ORDER)
		return 0;

	return reduce_to_order(curr_order, order);
}

/*
 * Send block to used list
 */
static void mark_block_used(struct page_block *pb)
{
}

/*
 * Returns NULL or 0 if fails
 */
void *page_alloc(unsigned int nr_pages)
{
	int order = 0;
	struct page_block *pb = 0;

	if (nr_pages > MAX_PAGE_BLOCK)
		return 0;

	order = get_page_order(nr_pages);
	pb = get_block(order);
	mark_block_used(pb);
	// TODO: map to va

	print_free_lists();
	return pb->page;
}
