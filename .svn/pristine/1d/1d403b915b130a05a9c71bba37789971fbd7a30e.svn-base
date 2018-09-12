/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */

// partner: Zhengru Qian: qzr2@illinois.edu

#include "assertions.h"
#include "disk.h"
#include "list.h"
#include "page_cache.h"
#include "page_fault_helpers.h"
#include <stdlib.h>

enum FaultResult load_shared_page(struct vm_area *vma, addr32 virt_address,
                                  struct page **out) {
   //step 1
   uint32_t page_offset = (virt_address - vma->start_addr) + vma->page_offset;
   struct mapped_file *f = vma->file;

   //step 2
   struct page *pg = page_cache_get(f->pathname, page_offset);

   if(pg) {
      *out = pg;
      return FAULT_MINOR;
   }

   //step 3
   pg = get_first_unused_page();
   if(!pg)
      return FAULT_NOMEM;

   //step 4
   zero_out_page_frame(pg);
   pg->mapping = f;
   pg->rmap = get_pte(vma->proc, virt_address, false);
   pg->file_offset = page_offset;
   if(-1 == read_page_from_disk(pg))
      return FAULT_BUS;

   //step 5
   page_cache_add(f->pathname, page_offset, pg);

   //page 7
   *out = pg;
   return FAULT_MAJOR;
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

   //clean page
   if(!page->dirty)
      return 0;

   //write fail
   if(-1 == write_page_to_disk(page))
      return -1;

   //step 2
   struct mapped_file *map = page->mapping;
   list_for_each(map->shared_vma_list, vm_mapped_next, elem) {

      //find process and virtual address
      struct process* p = elem->proc;

      //virt < end && elem->page_offset <= file_offset
      uint32_t virt = page->file_offset - elem->page_offset + elem->start_addr;

      if(virt < elem->end_addr && elem->page_offset <= page->file_offset) {

         //find PTE
         pte_t *page_table_entry = get_pte(p, virt, false);

         //mark PTE to "cleam"
         if(page_table_entry && page_table_entry->present)
            page_table_entry->dirty = 0;
      }

   }

   page->dirty = 0;
   page->private = 0;
   return 0;
}

int page_out_shared_page(struct page *page) {

   //step 1
   if(page->dirty && (-1 == write_page_to_disk(page)))
      return -1;

   //step 2
   struct mapped_file *map = page->mapping;
   list_for_each(map->shared_vma_list, vm_mapped_next, elem) {
      //find process and virtual address
      struct process* p = elem->proc;
      uint32_t virt = page->file_offset - elem->page_offset + elem->start_addr;

      if(virt < elem->end_addr && elem->page_offset <= page->file_offset) {
         //find PTE
         pte_t *page_table_entry = get_pte(p, virt, false);
         if(page_table_entry && page_table_entry->present) {
            //mark PTE to "not present"
            page_table_entry->present = 0;
            page_table_entry->dirty = 0;

            //reduce referance count
            page->reference_count--;
            if(!page->reference_count)
               break;
         }
      }
   }

   page_cache_remove(page);
   return 0;
}

int swap_out_private_page(struct page *pg) {
   uint32_t idx = get_free_swap_index();
   if(idx == (uint32_t)-1)
      return -1;
   pte_t *pte = pg->rmap;
   pte->present = 0;
   pte->dirty = 0;
   pte->base_addr = idx;
   if(-1 == write_page_to_swap(pte->base_addr, pg))
      return -1;
   return 0;
}
