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
#include "page_table.h"
#include "physical_memory.h"
#include "virtual_memory.h"

/**
 * Currently running process.
 */
struct process *current;

/**
 * Search through entire physical memory to try paging out a single page.
 * Returns the paged out `struct page` on success, or NULL on failure.
 */
static struct page *try_to_page_out_one(void);

/**
 * Try to page out a specific page; used as a helper function for
 * try_to_page_out_one.
 */
static int generic_page_out(struct page *);

/**
 * Return true iff the FaultResult is FAULT_SEGV, FAULT_BUS, or
 * FAULT_NOMEM.
 */
static bool bad_fault(enum FaultResult);

enum FaultResult resolve_page_fault(addr32 virt_addr, enum FaultReason why,
                                    bool write_op) {
    struct vm_area *vma = current->area_list;
    /* this loop handles the degenerate case of an empty VMA */
    do {
        vma = get_containing_vm_area(vma, virt_addr);
    } while (vma && vma->end_addr == vma->start_addr);

    enum FaultResult retval = FAULT_MINOR;
    pte_t *page_table_entry = NULL;

    /* First check to see if this virtual address is even valid. If not, we
     * can exit early with a segmentation violation
     */

    /* Virtual address not found in virtual address space */
    if (vma == NULL) {
        return FAULT_SEGV;
    }
    /* Illegal write */
    if (!vma->prot_write && write_op) {
        return FAULT_SEGV;
    }
    /* Illegal read */
    if (!vma->prot_read && !write_op) {
        return FAULT_SEGV;
    }
    assertion(why != PERMISSION,
              "MMU flagged permission error, but VMA says nothing is wrong?");

    page_table_entry = get_pte(current, virt_addr, true);

    if (!page_table_entry) { // No physical memory to make page table :(
        if (!try_to_page_out_one()) { // Failed to get memory by paging out!
            return FAULT_NOMEM;
        }
        page_table_entry = get_pte(current, virt_addr, true);
    }

    /* Need to mark page as dirty; only a minor fault */
    if (why == DIRTY) {
        struct page *pg = phys_to_page(page_table_entry->base_addr << 12);
        pg->dirty = 1;
        return FAULT_MINOR;
    }

    /**************************************************************************
     * All ensuing code involves loading page frames from somewhere and       *
     * assigning them to the PTE                                              *
     *************************************************************************/

    /* In case we need one, try to ensure an unused page is ready; for
     * simplicity we'll assume this is not a major fault  */
    struct page *pg = NULL;
    if (!(pg = get_next_unused_page(NULL))) {
        /* this might fail, but there's nothing we can do in that case */
        try_to_page_out_one();
    }

    /* this page was never allocated at all */
    if (why == NOT_PRESENT && !page_table_entry->available) {
        if (vma->map_anonymous) {
            assertion(vma->map_private,
                      "VMA was anonymous but not marked private!");
            /* No more memory; this must mean earlier attempt to page out
             * failed,
             * so no use trying again
             */
            if (!(pg = get_next_unused_page(NULL))) {
                return FAULT_NOMEM;
            }
            pg->private = 1;
            pg->rmap = page_table_entry;
            zero_out_page_frame(pg);
        } else if (vma->map_private) {
            retval = load_private_file_page(vma, virt_addr, &pg);
        } else {
            retval = load_shared_page(vma, virt_addr, &pg);
        }
    }

    /* page frame was allocated in the past, but now page data is stored on disk
     * somewhere
     */
    else if (why == NOT_PRESENT) {
        if (vma->map_anonymous) {
            assertion(vma->map_private,
                      "VMA was anonymous but not marked private!");
            retval = swap_in_private_page(vma, virt_addr, &pg);
        } else if (!vma->map_private) {
            retval = load_shared_page(vma, virt_addr, &pg);
        } else {
            retval = swap_in_private_page(vma, virt_addr, &pg);
        }
    }

    if (bad_fault(retval)) {
        return retval;
    }

    /**************************************************************************
     *     Invariant postconditions ensue                                     *
     **************************************************************************/

    ++pg->reference_count;
    page_table_entry->read_write = vma->prot_write;
    page_table_entry->available = 1;
    page_table_entry->present = 1;
    page_table_entry->user_supervisor = 1;
    page_table_entry->base_addr = page_to_phys(pg) >> 12;
    return retval;
}

void context_switch(struct process *proc) {
    if (proc == NULL) {
        current = NULL;
        return;
    }
    assertion(proc->pid != 0, "Process was not running!");
    current = proc;
}

static int generic_page_out(struct page *pg) {
    int retval =
        pg->private ? swap_out_private_page(pg) : page_out_shared_page(pg);
    if (!retval) {
        /* clear all flags and pointers in page; the page needs
           to already be removed from the appropriate linked lists
           in the specific page-out functions */
        *pg = (struct page){0};
    }
    return retval;
}

static struct page *try_to_page_out_one(void) {
    static struct page *most_recently_used = NULL;
    struct page *candidate = most_recently_used;
    do {
        candidate = get_next_swapout_candidate(candidate);
    } while (candidate && generic_page_out(candidate) == -1);

    if (!candidate && most_recently_used) {
        do {
            candidate = get_next_swapout_candidate(NULL);
        } while (candidate && generic_page_out(candidate) == -1);
    }

    return most_recently_used = candidate;
}

static bool bad_fault(enum FaultResult result) {
    return result == FAULT_SEGV || result == FAULT_BUS || result == FAULT_NOMEM;
}
