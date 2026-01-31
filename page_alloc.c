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
 * This is called at the very beginning and is responsible for initializing
 * the free_lists for all orders.
 */
static struct free_list_header fl_meta __attribute__((section(".phymem_meta")));
static unsigned int buddy_id_cntr;

static inline void print_page_block(struct page_block *pb)
{
	kprintf("addr:\t0x%x|\n id:\t%d| next:\t0x%x|", (u64)pb->page,
		pb->buddy_id,
							pb->next);
}

static struct page_block *new_page_block(void *page_addr)
{
	struct page_block *pb = fl_meta.last_entry;

	if (pb)
		pb++;
	else
		pb = (struct page_block *)
			((char *)&fl_meta + sizeof(struct free_list_header));

	pb->buddy_id = buddy_id_cntr++;
	pb->page = page_addr;
	pb->next = 0;

	fl_meta.last_entry = pb;
	return pb;
}

static void create_free_list(u64 start_addr, int order, size_t size)
{
	struct free_list *order_list = &fl_meta.list[order];
	struct page_block *cb = fl_meta.last_entry;
	u64 curr_addr = start_addr;
	size_t nr_blocks = size;

	if (!nr_blocks)
		return;

	while (nr_blocks--) {
		if (!cb)
			cb = new_page_block((void *)curr_addr);
		else
			cb->next = new_page_block((void *)curr_addr);

		if (!order_list->first)
			order_list->first = cb;

		order_list->count++;
		curr_addr += (PAGE_SIZE * PAGE_BLOCK(order));
	}
}

static inline size_t get_free_size(int order, unsigned int count)
{
	return (PAGE_BLOCK(order) * count * PAGE_SIZE);
}

void init_free_list(u64 addr, size_t size)
{
	unsigned int total_pages = size / PAGE_SIZE;
	int curr_order = MAX_PAGE_ORDER;
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
			blk_sz_kb = get_free_size(curr_order,
						  fl_meta.list[curr_order].count
						  );

			blk_sz_kb = blk_sz_kb / 1024;
			kprintf("O[%d]:\t%d blocks\t(%d KiB)\n",
				curr_order,
					fl_meta.list[curr_order].count,
					blk_sz_kb
			       );
		}

		curr_order--;
	}
}

void *page_alloc(unsigned int nr_pages)
{
	if (nr_pages > MAX_PAGE_BLOCK)
		return 0;

	return 0;
}
