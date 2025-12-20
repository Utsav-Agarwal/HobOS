#ifndef PT_LIB_H
#define PT_LIB_H

#include <stdint.h>

//-----
// Page Table Descriptor Attributes
//-----
#define PT_PAGE 	0b11        // 4k granule
#define PT_BLOCK	0b01        // 2M granule

#define PT_UXN_POS	54
#define PT_PXN_POS	53
#define PT_AF_POS	10
#define PT_SH_POS	8
#define PT_AP_POS	6
#define PT_NS_POS	5
#define PT_INDEX_POS	2

//AP Flag (Access permissions)

#define PT_KERNEL	(0b00 << PT_AP_POS)
#define PT_USER  	(0b01 << PT_AP_POS)
#define PT_AP_RW	(0b00 << PT_AP_POS)
#define PT_AP_RO	(0b10 << PT_AP_POS)

//AF Flag (Accessed desc)
#define PT_AF_ACCESSED	(1 << PT_AF_POS)

//UXN
#define PT_UXN_NX	(1UL << PT_UXN_POS)

//PXN
#define PT_PXN_NX	(1UL << PT_PXN_POS)

//SH Flag
#define PT_SH_O		(0b10 << PT_SH_POS)
#define PT_SH_I		(0b11 << PT_SH_POS)

//Indx Flag (index to MAIR_ELn)
#define PT_INDEX_MEM	(1 << PT_INDEX_POS)     // normal memory
#define PT_INDEX_DEV	(2 << PT_INDEX_POS)     // device MMIO
#define PT_INDEX_NC 	(0 << PT_INDEX_POS)     // non-cachable

//lets keep it 38 bits for now
#define PT_LVL_MIN	1
#define PT_LVL_MAX	3

//TODO: extend this for level 0, currently this is only
//till 38 bits
#define VADDR_MASK_4KB(level) \
	(0x5FC0000000UL >> (9*(PT_LVL_MIN-level)))

#define KB(x)	(x * (1 << 10))

#define PTE_FLAGS_GENERIC   \
    		(PT_PAGE | PT_AP_RW | PT_AF_ACCESSED \
		 | PT_SH_I | PT_INDEX_MEM)

#define PTE_FLAGS_IO   \
    		(PT_PAGE | PT_AF_ACCESSED \
		| PT_PXN_NX | PT_UXN_NX | PT_SH_O | PT_INDEX_DEV)

#define PTE_FLAGS_NC   \
    		(PT_PAGE | PT_AF_ACCESSED \
		| PT_PXN_NX | PT_UXN_NX | PT_SH_O | PT_INDEX_NC)

//the actual structures are on the heap, appended just under the tables
//that they describe. to retrieve them, just read after the last table entry

//TODO: add roundoff
//#define NEXT_PT_OFFSET (512*8 + sizeof(struct page_table_desc))
#define NEXT_PT_OFFSET (0x2000)

// a list of entries need to be tracked for
// every thread when we want to track context.
// Upon completion of the same, we can now cleanup/free
// the memory in question.
struct t_desc {
	uint64_t vaddr;
	uint64_t paddr;
	uint16_t flags;
	struct uint64_t *entry;
	struct uint64_t *next;
	struct uint64_t *prev;
};

// we can have a list of page_table_descs
// that define the page tables in use
struct page_table_desc {
	uint8_t level;
	volatile uint64_t *pt;
	uint16_t pt_len;	//max 512 elements, 9bits each level
};

struct va_metadata {
	uint64_t index[3];
	uint64_t offset;
};

//extract pte desc/meta data - more for debugging purposes
//struct *uint64_t_desc get_pte(void *);

uint64_t pt_entry(uint64_t paddr, uint64_t flags);
void place_pt_entry(struct page_table_desc *pt_desc, uint64_t pte, int index);
struct page_table_desc *create_pt(uint64_t pt_baddr, uint8_t level);
uint64_t *create_pt_entries(
		struct page_table_desc *pt_desc,
		uint64_t start_paddr, uint64_t end_paddr, uint64_t flags);

//TODO: remove
void reserve_block(uint64_t baddr);

#endif
