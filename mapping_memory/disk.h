/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "types.h"
/**
 * This header contains methods used to load data from and store data to disk.
 *
 * The two forms of disk access are file-based and swap-based.
 *
 * File-based IO occurs through a `struct mapped_file` object, to which every
 * file-backed, shared page points. In other words, each file mapped by any
 * process or present in the disk cache has a single `struct mapped_file`, and
 * all shared `struct page`s backed by this file point to this `struct
 * mapped_file`.
 *
 * Swap-based IO involves a simple transfer to a transparent, monolothic,
 * index-based disk partition. Anonymous and private pages are stored at
 * the first available "swap slot" with a given "swap index". When a process'
 * page gets swapped out to a swap slot, the swap index is stored in the
 * corresponding page table entry's `base_addr` field. The page fault
 * handler retrieves the swap index from the PTE and uses this to load the
 * appropriate data from swap to a page frame.
 */

struct mapped_file {
    /* Name of file */
    char *pathname;
    /*  Linked list of all VMAs that form shared mappings to this file;
     *  useful when reverse mapping to page out a file-backed, shared page */
    struct vm_area *shared_vma_list;
    /* Number of VMAs (private and shared) that use this file */
    uint32_t reference_count;
};

/**
 * Obtains the `struct mapped_file` corresponding to this pathname. If no such
 * struct already exists, one will be created. If the pathname is invalid,
 * then this returns NULL.
 */
struct mapped_file *obtain_mapped_file(char const *pathname);

/**
 * Returns the index of a free swap slot. If there are no more free swap slots,
 * this functions returns `(uint32_t) -1`.
 */
uint32_t get_free_swap_index(void);

/**
 * Marks the swap slot at the given `index` as unused. This should be used
 * whenever a private page gets unmapped while swapped out.
 */
int free_swap_slot(uint32_t index);

/**
 * Writes the page frame of physical memory represented by the given
 * `struct page` to the given swap index. This will also mark the swap slot as
 * used. However, it does nothing else.
 *
 * If the given swap slot is already being used, returns -1. Returns 0
 * otherwise.
 */
int write_page_to_swap(uint32_t index, struct page *);

/**
 * Reads the contents of swap stored at the indexed swap slot into the physical
 * page frame represented by the given `struct page`. This will also mark the
 * swap slot as unused. However, it does nothing else.
 *
 * If the given swap slot is not being used, returns -1. Returns 0 otherwise.
 */
int read_page_from_swap(uint32_t index, struct page *);

/**
 * Given a `struct page` representing a physical frame that backs a
 * memory-mapped  file, this function will read however many bytes are left in
 * the file or the entire page frame size, whichever is smaller, into that
 * page frame.
 *
 * Note that this requires the `mapping` and `offset` components of the
 * `struct page` to be set properly.
 *
 * Returns -1 if this operation fails.
 */
int read_page_from_disk(struct page *);

/**
 * Given a `struct page` representing a physical frame that backs a
 * memory-mapped file, this function will write however many bytes are left in
 * the file or the entire page frame size, whichever is smaller, into that
 * file.
 *
 * Note that this requires the `mapping` and `offset` components of the
 * `struct page` to be set properly.
 *
 * Returns -1 if this operation fails. The page fault handler should respond
 * to this event by returning a bus error.
 */
int write_page_to_disk(struct page *);
