// SPDX-License-Identifier: GPL-2.0-only

#include <hobos/page_alloc.h>

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
 *
 */
void *page_alloc(unsigned int nr_pages)
{
	if (nr_pages > MAX_PAGE_BLOCK)
		return 0;

	return 0;
}
