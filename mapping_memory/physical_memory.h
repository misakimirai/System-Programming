/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "types.h"
/**
 * Book-keeping for a physical frame. All page structs are statically
 * initialized when the system boots up, which is why there are no "create"
 * or "destroy" methods. After all, physical memory is never created or
 * destroyed on a running system.
 *
 * For simplicity, we are going to assume that these pages only cover userspace
 * pages, page directories, and page tables. The rest of the memory, including
 * the memory of the page structs itself, exists in some magical kernel virtual
 * address space, with magical book-keeping.
 */
struct page {
    /* counts number of processes using this page frame; if this hits 0, it is
     * free (unless the reserved bit is also set)
     */
    uint32_t reference_count;

    /* flags */
    uint32_t reserved : 1; // page used for page table / directory
    uint32_t dirty : 1;    // page is dirty
    uint32_t private : 1;  // page is private
    uint32_t unused : 29;  // not important

    /* For private pages, this will reference the unique PTE that maps to this
     * page, and is essential for swapping them out */
    pte_t *rmap;

    /* depicts file resource used by this page; NULL if page frame is not
     * backed by a file
     */
    struct mapped_file *mapping;

    /* represents page-aligned offset within file, denoting which portion of the
     * file backs this page frame; this is only meaningful if the page frame is
     * backed by a file
     */
    uint32_t file_offset;

    /* linked list pointers for use in page cache hash table; NULL if page
     * frame isn't backed by a file */
    struct page *hash_next;
    struct page **hash_prev;
};

/**
 * Returns a `struct page` representing an unused page frame. This is
 * determined by looking at the `reference_count` and `reserved` fields; if
 * `reference_count` is not 0, or `reserved` is not 0, then the
 * corresponding page frame is considered to be in use.
 *
 * Note that the resulting `struct page` itself will be zeroed out, but the
 * associated page frame might not be. Generally, the page frame should be
 * zeroed out after acquisition.
 *
 * Use `get_next_unused_page(NULL)` to get the page for the first unused page
 * frame in physical memory.
 *
 * Using this method multiple times will keep yielding free pages until none
 * remain.
 *
 * Returns NULL if there are no free pages left.
 */
struct page *get_next_unused_page(struct page *current_page);

/**
 * Equivalent to `get_next_unused_page(NULL)`. Gets the first unused
 * `struct page` representing the first page frame in physical memory.
 */
struct page *get_first_unused_page(void);

/**
 * Gets the next page available for being swapped out / paged back to disk,
 * not including this one.
 */
struct page *get_next_swapout_candidate(struct page *curr);

/**
 * Zeros out the page frame represented by a `struct page`. Note that this
 * is not the same as zeroing out the `struct page` itself, i.e. this won't
 * modify any of the members of the `struct page`.
 */
void zero_out_page_frame(struct page *);

/**
 * Copies over memory from one page frame to another. Note that this does
 * not actually copy the contents of the `struct page`, but rather the
 * physical memory represented thereby.
 */
void copy_page_frame(struct page *dest, struct page *src);

/**
 * Converts a physical address to a pointer to the `struct page` that manages
 * it.
 */
struct page *phys_to_page(addr32 phys_addr);

/**
 * Gets the page-aligned physical address referring to the start of the
 * physical memory represented by a given `struct page`.
 */
addr32 page_to_phys(struct page *);

/**
 * Gets a "kernel-space pointer" to the physical memory represented by this
 * physical address. You do not have to use this method.
 */
void *get_system_pointer(addr32 phys_addr);

/**
 * Converts a system pointer to a physical address; note that this will only
 * work on pointers that map objects in the physical memory subsystem
 * (i.e. page tables pointers, page directory pointers, userspace page frame
 * pointers).
 */
addr32 system_pointer_to_phys(void *);
