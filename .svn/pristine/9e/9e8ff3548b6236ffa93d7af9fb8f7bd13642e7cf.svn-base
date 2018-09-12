/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "kernel.h"
#include "physical_memory.h"
#include "types.h"
#include "virtual_memory.h"

/**
 * Helper functions for the kernel page fault handler.
 *
 * Note that any `struct page` that gets involved in a page out / swap out
 * to disk will have all of its flags cleared by the page fault handler,
 * so you don't need to do this.
 *
 * Furthermore, after you load pages from disk or the page cache, the page
 * fault handler will increment `struct page->reference_count` for you, and
 * set the right PTE's  base address to refer to base address of the
 * `struct page`.
 *
 * `get_first_unused_page` and `get_next_unused_page` will clear all the flags
 * of any `struct page` they return.
 */

/**
 * Page out shared page to disk to save physical memory. This involves doing
 * the following:
 *
 * 1. Write the `struct page` back to disk if it is dirty (see disk.h)
 * 2. Find all VMAs that use this page using object-based reverse mapping
 *    (read below to know what that means)
 * 3. For each such VMA, if the VMA maps this page
 *    a. find the `struct process` and virtual address corresonding to this
 *       page
 *    b. find the PTE corresponding to this process + virtual address pair,
 *       if it exists and is "present" (if not, skip to the next VMA)
 *    c. mark the PTE as "not present" and "clean", i.e. clear the `present`
 *       and `dirty` bits
 *    d. reduce the reference count of the `struct page` by 1; if the reference
 *       count hits zero, you can assume no more VMAs map the page and stop
 * 4. Remove the `struct page` from the page cache
 * 5. Clear all flags in the `struct page`
 *
 * ERROR HANDLING:
 *  - In the event of a bus error (`write_page_to_disk` fails) at step 1,
 *    return -1
 *  - You may assume that the `struct page` actually maps a file (i.e.
 *    `mapped_file` is not NULL)
 *  - Assume everything else works as expected
 *
 * This function requires a technique known as object-based reverse-mapping.
 * This was merged into Linux 2.5 in February 25, 2003, and is explained
 * at a high level here:
 *
 * https://lwn.net/Articles/23732/
 *
 * This might require a bit of effort to understand. In essence, each `struct
 * page` that is backed by a file points to the respective `mapped_file` and
 * describes the offset in the file that it maps. In turn, that `mapped_file`
 * points to every VMA in every process that (shared) maps the file in member
 * `mapped_file->shared_vma_list`. By iterating through every such VMA in that
 * list, you can check to see if the specific page is actually mapped
 * by the VMA. If so, you can compute the virtual address in that VMA that maps
 * to said page in the file, and from there get the PTE, and from there mark
 * the PTE as "not present"
 *
 */
int page_out_shared_page(struct page *);

/**
 * Sync the shared page to disk. This is very much like `page_out_shared_page`,
 * except for a couple differences:
 *  -reference counts are not decreased on the `struct page`, since the page
 *  frame is still valid and in-use
 *  -the `struct page` is not removed from the page cache for the same reason
 *  -PTEs are marked as "clean" (i.e. clear the `dirty` bit) rather than
 *  "not present"
 *  -if the `struct page` is not dirty, you can return immediately with
 *  success, since no PTE can possibly be dirty when no one dirtied the page
 *
 * This is used by the `msync` system call.
 *
 * ERROR HANDLING: Same as `page_out_shared_page`
 */
int sync_shared_page(struct page *);

/**
 * This is essentially the inverse of `swap_out_private_page`.
 *
 * 1. Get an unused page `pg`.
 * 2. Get the PTE corresponding to the given virtual address in the current
 *    process.
 * 3. Get the `base_addr` field of the PTE and interpret as a swap index,
 *    and use `read_page_from_swap` to load data from that swap index to `pg`
 *    (see disk.h).
 * 4. Set `rmap` to the right PTE, and mark the page as private.
 * 4. Get `out` to point to `pg`.
 * 5. Return FAULT_MAJOR.
 *
 * ERROR HANDLING:
 *  - In the event that step 1 fails, return FAULT_NOMEM.
 *  - If `read_page_from_swap` fails, return FAULT_BUS.
 */
enum FaultResult swap_in_private_page(struct vm_area *vma, addr32 virt_addr,
                                      struct page **out);

/**
 * Load a shared (file-backed) page from disk and store the result in
 * the pointer `out`. This requires the following operations:
 *
 * 1. Use the VMA to get the filename and page offset. This is a bit tricky.
 * 2. Check to see if the (filename, page offset) already corresponds to a page
 *    in the page cache. If so, store this page in `out` and return.
 * 3. Try to get an unused page using `get_first_unused_page`.
 * 4. Load the proper information into the `mapped_file` section of newly
 *    acquired `struct page` and use `read_page_from_disk()` to get the file
 *    data in physical memory.
 * 5. Insert this new page into the page cache with the right filename and
 *    offset arguments.
 * 6. Set the appropriate `struct page` flags and increment its reference
 *    count.
 * 7. Store this new page in `out`.
 *
 * ERROR HANDLING:
 *  -If the page is found in the page cache in step 2, return FAULT_MINOR
 *  -If step 3 fails to give an unused page frame, return FAULT_NOMEM
 *  -If step 4 fails at `read_page_from_disk()`, return FAULT_BUS
 *  -Return FAULT_MAJOR if step 4 succeeded.
 *
 * Hints:
 *  -`vma->start_addr` corresponds to the offset `vma->page_offset`
 *   _in the file_. So what offset does `vma->start_addr + 4096` correspond to
 *   _in the file_?
 *  -All offsets have to be page-aligned, even if the virtual address is not
 *  -Is the freshly loaded `struct page` dirty? Is it going to be private?
 */
enum FaultResult load_shared_page(struct vm_area *vma, addr32 virt_address,
                                  struct page **out);

/**
 * This is identical to `load_shared_page()`, but the process gets a private
 * copy of the file-backed page instead of getting a reference to a page in
 * the page cache. Also, if a file-backed page isn't currently present in
 * the page cache, this will not insert any page into the page cache.
 */
enum FaultResult load_private_file_page(struct vm_area *vma,
                                        addr32 virt_address, struct page **out);

/**
 * Page out private page to swap out to save physical memory. This involves
 * doing the following:
 *
 * 1. Find a free swap index (let's call it `idx`)
 * 2. Find the PTE that maps to this `struct page` (see the `page->rmap` field).
 * 3. Mark the PTE as "not present" and "clean"
 * 4. Set the `base_addr` field of the PTE to `idx`
 * 5. Write the `struct page` to swap at `idx` (see disk.h)
 *
 * ERROR HANDLING:
 *  - In the event that there is no free swap space at step 1, return -1
 *  - Assume that the page is actually a private page, and that `page->rmap`
 *  is not NULL
 */
int swap_out_private_page(struct page *);
