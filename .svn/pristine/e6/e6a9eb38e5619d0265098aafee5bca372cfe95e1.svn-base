/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "disk.h"
#include "kernel.h"
#include "list.h"
#include "page_cache.h"
#include "page_fault_helpers.h"
#include "physical_memory.h"
#include "syscalls.h"
#include "types.h"
#include "virtual_memory.h"

/**
 * The mmap system call creates a virtual memory area (VMA) in the current
 * process' address space and dedicates this VMA to a given file, setting the
 * appropriate flags in the VMA. Note that no two VMAs in the same process can
 * have overlapping virtual address ranges, and the virtual address range must
 * be exactly `length` bytes large.
 *
 * If the `pathname` is not NULL and MAP_ANONYMOUS is not set, the syscall
 * should also obtain a `mapped_file` struct for the pathname (hint: look in
 * `disk.h`),
 * increment its reference count, and set the VMA's `file` member to that
 * `mapped_file`. The
 * VMA `page_offset` should be set to `offset`.
 * Additionally, if MAP_SHARED is is set, the syscall pushes the VMA to the
 * front of the `file->shared_vma_list` linked list.
 *
 * `length`: size of the newly created VMA. must be page-aligned.
 *    If the VMA is mapped to a file, note that it is legal to map more pages
 *    than remain in the file, but doing so
 *    will result in a bus error when those bytes are accessed.
 * `prot_flags`: can be some combination of PROT_READ, PROT_WRITE, PROT_EXEC,
 *    and PROT_NONE. See `struct vm_area->flags`.
 * `flags`:  must be MAP_SHARED or MAP_PRIVATE, but not both, and possibly
 *    MAP_ANONYMOUS, which indicates that the VMA is not backed by any file.
 *    MAP_SHARED and MAP_ANONYMOUS cannot both be set. See `vm_area->prot_flags`
 * `pathname`: name of the file to be mapped; this is ignored if MAP_ANONYMOUS
 *    is set
 * `offset`: page-aligned offset in file from which the mapping starts; e.g.
 *    an offset of 4096 means that the virtual address returned maps to the
 *    4096th byte of the file. This is ignored if MAP_ANONYMOUS is set.
 *
 * Returns the page-aligned virtual address of the beginning of the mapped
 * region if the system call succeeds, otherwise returns MAP_FAILED.
 *
 * ERROR HANDLING:
 * -Invalid flag or argument combinations
 * -Unable to obtain `mapped_file` struct for the pathname when it is needed
 * -Integer arguments not page aligned
 */
uint32_t syscall_mmap(uint32_t length, int prot_flags, int flags,
                      char const *pathname, uint32_t offset) {
    return MAP_FAILED;
}

/**
 * Shifts the program break (i.e. the end of the heap) by the given positive
 * offset. The offset must be page-aligned.
 *
 * On success, returns the new program break virtual address. Returns
 * MAP_FAILED on failure.
 */
addr32 syscall_sbrk(uint32_t shift) {
    return MAP_FAILED;
}

/**
 * If `addr` corresponds to a shared page backed by a memory-mapped file,
 * that page will be synced (written back) to disk if dirty (see disk.h).
 *
 * Returns 0 on success. Returns -1 if `addr` isn't in the current process'
 * virtual address space or if `addr` does not correspond to a shared,
 * memory-mapped file.
 */
int32_t syscall_msync(addr32 virt_addr) {
    virt_addr = align(virt_addr, NUM_OFFSET_BITS);

    struct vm_area *vma = get_containing_vm_area(current->area_list, virt_addr);

    // Error: Virtual address does not exist in process address space
    if (vma == NULL) {
        return -1;
    }

    // Error: does not make sense to sync private pages; they are process-local
    if (vma->map_private) {
        return -1;
    }

    // Compute offset of this particular page in the file
    uint32_t file_offset = (virt_addr - vma->start_addr) + vma->page_offset;

    // Search for page within page cache
    struct page *pg = page_cache_get(vma->file->pathname, file_offset);

    // Call subroutine (see page_fault_helpers.h) to check to see if the page is
    // dirty and, if so, perform reverse mapping to set every PTE from every
    // process that uses it to "clean"
    return sync_shared_page(pg);
}

/*******************************************************************************
 *                                                                             *
 * The following is provided to you, so and you should not have to make any    *
 * changes. Reading the code might provide a lot of insight, however.          *
 *                                                                             *
 ******************************************************************************/

/**
 * The munmap system call simply destroys a VMA in the current process'
 * address space that contains the given virtual address. In doing so, it must
 * also unmap all PTEs and (if no other VMAs use a given page table)
 * all page tables corresponding to this VMA.
 *
 * All present `struct page`s mapped by this VMA need to have their reference
 * counts decremented. If the VMA represents anonymous or private memory,
 * every not-present page present in swap needs to have its swap slot marked as
 * free. Note the `present` bit of a PTE denotes whether a page is present.
 *
 * The `struct page` corresponding to a virtual address
 * can be found by looking at the `base_addr` of the corresponding page table
 * entry (if its present flag is set) and using `phys_to_page` (see
 * physical_memory.h).
 *
 * `addr`: virtual address corresponding to a VMA
 *
 * Returns 0 on success and -1 if no such virtual address exists in the process
 * address space.
 */
int32_t syscall_munmap(uint32_t addr) {

    // Find the VMA that contains this virtual address
    struct vm_area *vma = get_containing_vm_area(current->area_list, addr);

    // Error: virtual address not in process address space
    if (vma == NULL) {
        return -1;
    }

    // Unlink VMA from all its linked lists
    unlink_vm_area(vma);

    const uint32_t pde_off_bits = NUM_OFFSET_BITS + NUM_VPN_BITS;
    for (addr32 virt_addr = vma->start_addr, next_addr = 0;
         virt_addr < vma->end_addr; virt_addr = next_addr) {
        // refers to virtual address referenced by next PDE
        next_addr = align(virt_addr + pow2(pde_off_bits), pde_off_bits);

        pde_t *directory_entry = get_pde(current, virt_addr);

        // directory entry isn't in use, so skip ahead to next PDE
        if (!directory_entry->available) {
            virt_addr = next_addr;
            continue;
        }
        // now iterate through all PTEs under this PDE
        for (; virt_addr < MIN(vma->end_addr, next_addr);
             virt_addr += PAGE_SIZE) {
            pte_t *page_table_entry = get_pte(current, virt_addr, false);

            assertion(page_table_entry,
                      "The page table is known to exist, so this "
                      "entry should also exist. Error!");
            // PTE was never used, so skip this step
            if (!page_table_entry->available) {
                continue;
            }

            if (!page_table_entry->present) {
                // if page is not present, it only matters if the page is
                // private, in
                // which case something is present in swap and must be freed
                if (vma->map_private) {
                    assertion(
                        free_swap_slot(page_table_entry->base_addr) == 0,
                        "Swap slot was not even in use, despite combination of "
                        "`present` and `available` flags.");
                }
            } else {
                // if PTE is present, we need to dereference a struct page in
                // any case
                struct page *pg = phys_to_page(page_table_entry->base_addr
                                               << NUM_OFFSET_BITS);
                --pg->reference_count;

                // if the VMA is shared and file-backed, we may need to do some
                // IO
                if (!vma->map_anonymous && !vma->map_private) {
                    assertion(pg->mapping,
                              "Shared, file-backed VMA somehow referenced "
                              "a `struct page` with a NULL file mapping.");
                    // if struct page is dirty, mark it clean
                    if (pg->dirty) {
                        pg->dirty = 0;
                        write_page_to_disk(pg);
                    }
                    // if the reference count hit 0, remove it from the page
                    // cache
                    if (!pg->reference_count) {
                        page_cache_remove(pg);
                    }
                }
            }
            // clear page table entry, marking it as unused
            clear_pte(page_table_entry);
        } // END iterating through all PTEs of given page table

        // page table might now be unused and ready to be freed
        if (!page_table_in_use(directory_entry)) {
            struct page *page_table_page =
                phys_to_page(directory_entry->base_addr << NUM_OFFSET_BITS);
            page_table_page->reserved = 0;
        }
    }

    // Decrement reference count of file if not anonymous and free VMA
    if (!vma->map_anonymous) {
        --vma->file->reference_count;
    }
    free_vm_area(vma);
    return 0;
}
