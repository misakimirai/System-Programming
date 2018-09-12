/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "types.h"

/*
 * The following structs are bit for the standard 32bit virtual and physical
 * address,
 * 4kb offset, 2 level page table.
 */
struct page_directory_entry {
    uint32_t base_addr : BASE_ADDR_SIZE;
    uint32_t available : 3;
    uint32_t global_page : 1;
    uint32_t page_size : 1;
    uint32_t reserved : 1;
    uint32_t accessed : 1;
    uint32_t cache_disabled : 1;
    uint32_t write_through : 1;
    uint32_t user_supervisor : 1;
    uint32_t read_write : 1;
    uint32_t present : 1;
};

struct page_directory {
    struct page_directory_entry entries[NUM_ENTRIES];
};

struct page_table_entry {
    /* when a page is swapped out to disk, this provides the swap index of that
     * page; when a page is present in main memory, this provides the BASE
     * physical address of that page frame (see `phys_to_page`)
     */
    uint32_t base_addr : BASE_ADDR_SIZE;
    /*
     * if (none of these) flags are set, then no page frame was ever allocated
     * for this page, so the kernel must provide an unused page frame
     */
    uint32_t available : 3;
    uint32_t global_page : 1;
    uint32_t page_table_attribute_index : 1;
    /*
     * indicates that a page has been written to; causes a page fault when
     * a write_op is performed on this PTE's frame and the flag is cleared,
     * in which case hardware will clear the flag and software must mark the
     * corresponding `struct page` as dirty (if it isn't already)
     */
    uint32_t dirty : 1;
    uint32_t accessed : 1;
    uint32_t cache_disabled : 1;
    uint32_t write_through : 1;
    uint32_t user_supervisor : 1;
    uint32_t read_write : 1;
    /* if this flag is not set, but `accessed` is set, then the page was swapped
     * out to disk, or paged out to a memory-mapped file */
    uint32_t present : 1;
};

struct page_table {
    struct page_table_entry entries[NUM_ENTRIES];
};

union paging_entry {
    struct page_directory_entry pde;
    struct page_table_entry pte;
};

/**
 * Gets the PTE corresponding to the given virtual address under the given
 * process. If `allocate_page_table` is set, this method transparently
 * allocates page tables if none have yet been allocated for the given virtual
 * address. It returns NULL if `allocate_page_table` is not set and the
 * corresponding page table has not yet been allocated, or if the system is
 * out of memory and cannot allocate new page tables.
 *
 * `allocate_page_table` should only be set when using this function directly
 * the page fault handler, which is responsible for allocating page tables.
 * You should probably never use this function.
 */
pte_t *get_pte(struct process *proc, uint32_t virt_addr,
               bool allocate_page_table);

/**
 * Clears a page table entry completely, marking it as unused. This is only
 * used in the implementation of `syscall_munmap`.
 */
void clear_pte(pte_t *page_table_entry);

/**
 * Clears a page directory entry, marking it as unused. This is only used in
 * the implementation of `syscall_munmap`.
 */
void clear_pde(pde_t *page_directory_entry);

/**
 * Checks to see if a page table is actually being used. This is only used
 * in the implementation of `syscall_munmap`.
 */
bool page_table_in_use(pde_t *page_directory_entry);

/**
 * Gets the page table directory entry corresponding to the given virtual
 * address in the given process' virtual address space. Recall that this
 * is an entry that gives the physical base address of a page table, if an
 * `available` bit is set.
 *
 * This method should only be used when unmapping page tables, i.e. in
 * `syscall_munmap` or in `kill_process`.
 *
 * You should probably never use this function.
 */
pde_t *get_pde(struct process *proc, uint32_t virt_addr);

/*
 * The following is documentation for all the fields for those that are curious
 * (IA-32 Intel® Architecture Software Developer’s Manual, Volume 3)
 * (http://flint.cs.yale.edu/cs422/doc/24547212.pdf#page=88)
 */
