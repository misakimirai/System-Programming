/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "list.h"
#include "page_table.h"
#include "physical_memory.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define assert_page_proper(pg)                                                 \
    {                                                                          \
        if (((uint64_t)pg - (uint64_t)&mem_map) % sizeof(struct page)) {       \
            fprintf(stderr, "%s: Misaligned struct page pointer!\n",           \
                    __FUNCTION__);                                             \
            raise(SIGABRT);                                                    \
        } else if (pg < mem_map || pg - mem_map >= NUM_PHYSICAL_PAGES) {       \
            fprintf(stderr, "%s: Struct page pointer out of bounds!\n",        \
                    __FUNCTION__);                                             \
            raise(SIGABRT);                                                    \
        }                                                                      \
    }

#define assert_addr_proper(phys_addr)                                          \
    if (phys_addr > PHYSICAL_MEMORY_SIZE) {                                    \
        fprintf(stderr, "%s: physical address out of bounds!\n",               \
                __FUNCTION__);                                                 \
        raise(SIGABRT);                                                        \
    }

/* global array of all page structs */
static struct page mem_map[NUM_PHYSICAL_PAGES];

/* global physical memory; in reality, mem_map is a part of physical memory...
 */
static char physical_memory[PHYSICAL_MEMORY_SIZE];

struct page *get_next_unused_page(struct page *curr) {
    for (struct page *pg = curr ? curr + 1 : mem_map;
         pg < mem_map + NUM_PHYSICAL_PAGES; ++pg) {
        if (!pg->reserved && !pg->reference_count) {
            /* remove from page cache and zero out flags */
            if (pg->hash_prev || pg->hash_next) {
                list_unlink(pg, hash_prev, hash_next);
            }
            /* zero out the struct, so as to not cause confusion */
            *pg = (struct page){0};
            return pg;
        }
    }
    return NULL;
}

struct page *get_first_unused_page(void) {
    return get_next_unused_page(NULL);
}

struct page *get_next_swapout_candidate(struct page *curr) {
    curr = curr ? curr + 1 : mem_map;
    assert_page_proper(curr);
    for (struct page *pg = curr + 1; pg < mem_map + NUM_PHYSICAL_PAGES; ++pg) {
        if (!pg->reserved && pg->reference_count) {
            return pg;
        }
    }
    return NULL;
}

void zero_out_page_frame(struct page *pg) {
    assert_page_proper(pg);
    // basically hardcoded memset (on 32-bit architecture), assuming aligned
    memset(physical_memory + page_to_phys(pg), 0, FRAME_SIZE);
}

void copy_page_frame(struct page *dest, struct page *src) {
    assert_page_proper(dest);
    assert_page_proper(src);
    // basically hardcoded memcpy (on 32-bit architecture), assuming aligned
    uint32_t *phys_src = (uint32_t *)(physical_memory + page_to_phys(src));
    uint32_t *phys_dest = (uint32_t *)(physical_memory + page_to_phys(dest));
    for (uint32_t i = 0; i < FRAME_SIZE / sizeof(uint32_t); ++i) {
        phys_dest[i] = phys_src[i];
    }
}

struct page *phys_to_page(addr32 phys_addr) {
    assert_addr_proper(phys_addr);
    return mem_map + (phys_addr >> NUM_OFFSET_BITS);
}

union paging_entry *get_paging_entry(addr32 page_directory_or_table_addr,
                                     uint32_t index) {
    assert_addr_proper(page_directory_or_table_addr);
    struct page *pg = phys_to_page(page_directory_or_table_addr);
    if (!pg->reserved) {
        fprintf(stderr,
                "%s: Passed in physical address of userspace page frame "
                "instead of page table (reserved bit was not set)!\n",
                __FUNCTION__);
        raise(SIGABRT);
    }
    if (index >= NUM_ENTRIES) {
        fprintf(stderr, "%s: Invalid paging entry index!\n", __FUNCTION__);
    }
    return ((union paging_entry *)(physical_memory +
                                   (page_to_phys(pg) << NUM_OFFSET_BITS))) +
           index;
}

addr32 page_to_phys(struct page *pg) {
    assert_page_proper(pg);
    return ((addr32)(pg - mem_map)) << NUM_OFFSET_BITS;
}

void *get_system_pointer(addr32 phys_addr) {
    return physical_memory + phys_addr;
}

addr32 system_pointer_to_phys(void *sys_ptr) {
    return (addr32)((char *)sys_ptr - physical_memory);
}
