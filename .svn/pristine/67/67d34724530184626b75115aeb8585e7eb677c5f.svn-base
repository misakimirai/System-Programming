/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "disk.h"
#include "list.h"
#include "page_cache.h"
#include "page_fault_helpers.h"
#include <stdlib.h>

enum FaultResult load_shared_page(struct vm_area *vma, addr32 virt_address,
                                  struct page **out) {
    return FAULT_BUS;
}

enum FaultResult load_private_file_page(struct vm_area *vma,
                                        addr32 virt_address,
                                        struct page **out) {
    // calculate offset witin file from VMA details and virtual address
    uint32_t page_offset = (virt_address - vma->start_addr) + vma->page_offset;

    // getting a private page always requires one new page frame
    struct page *pg = get_first_unused_page();
    if (!pg) {
        return FAULT_NOMEM;
    }
    zero_out_page_frame(pg);
    pg->private = 1;
    pg->mapping = vma->file;
    pg->file_offset = page_offset;
    pg->rmap = get_pte(vma->proc, virt_address, false);

    assertion(pg->rmap, "Precondition asserts PTE must exist for this page.\n");

    struct page *search = NULL;
    // found it in the page cache, so we make a private copy and we're good
    if ((search = page_cache_get(pg->mapping->pathname, page_offset))) {
        copy_page_frame(pg, search);
        *out = pg;
        return FAULT_MINOR;
    }

    // did not find it, so we need to read it directly from disk
    if (read_page_from_disk(pg) == -1) {
        return FAULT_BUS;
    }

    *out = pg;
    return FAULT_MAJOR;
}

enum FaultResult swap_in_private_page(struct vm_area *vma, addr32 virt_address,
                                      struct page **out) {
    pte_t *page_table_entry = get_pte(vma->proc, virt_address, false);
    assertion(page_table_entry && page_table_entry->available,
              "Preconditions forbid a non-available PTE.");
    // get one unused page frame
    struct page *pg = get_first_unused_page();
    // Error: no physical memory left
    if (!pg) {
        return FAULT_NOMEM;
    }
    // Error: failed to read from swap, somehow
    if (read_page_from_swap(page_table_entry->base_addr, pg) == -1) {
        return FAULT_BUS;
    }
    pg->rmap = page_table_entry;
    pg->private = 1;

    // technically this is not necessary for private pages
    pg->mapping = vma->file;
    pg->file_offset = vma->page_offset;

    *out = pg;
    return FAULT_MAJOR;
}

int sync_shared_page(struct page *page) {
    return -1;
}

int page_out_shared_page(struct page *page) {
    return -1;
}

int swap_out_private_page(struct page *pg) {
    return -1;
}
