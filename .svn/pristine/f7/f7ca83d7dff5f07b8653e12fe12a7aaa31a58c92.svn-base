/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "kernel.h"
#include "nommu.h"
#include "page_table.h"
#include "physical_memory.h"
#include <string.h>

/**
 * Helper function that attempts to write or read at most one entire page from
 * memory at once, handling edge cases where the starting virtual address or
 * ending virtual address is not page aligned.
 *
 * `orig_addr`:
 *    Original 32-bit address passed into `nommu_access_virtual_address`
 * `current_addr`:
 *    Page-aligned 32-bit address denoting start of current page
 *    on which to perform (possibly partial) access
 * `length`:
 *    Original length passed into `nommu_access_virtual_address`
 * `system_pointer`:
 *    System pointer to the physical address where the currently
 *    accessed page frame starts
 * `user_buffer`:
 *    Correctly offset buffer [from which / to which] data is currently
 *    [being read / being written], depending on `write_op`
 * `write_op`:
 *    True iff this operation is performing a write
 */
static uint32_t exchange_bytes(addr32 orig_addr, addr32 current_addr,
                               uint32_t length, void *physical_system_pointer,
                               void *user_buffer, bool write_op);

enum FaultResult nommu_access_virtual_address(addr32 virt_addr, void *buffer,
                                              uint32_t length, bool write_op) {
    enum FaultResult retval = FAULT_NONE;
    addr32 aligned_addr = page_align(virt_addr);
    for (; aligned_addr < virt_addr + length; aligned_addr += 4096) {
        pte_t *pte = NULL;
        enum FaultResult res = FAULT_NONE;
    TRY_ACCESS_AGAIN:
        res = FAULT_NONE;
        pte = get_pte(current, aligned_addr, false);
        if (!pte || !pte->present) {
            res = resolve_page_fault(aligned_addr, NOT_PRESENT, write_op);
        } else if (!pte->user_supervisor) {
            res = resolve_page_fault(aligned_addr, PERMISSION, write_op);
        } else if (!pte->read_write && write_op) {
            res = resolve_page_fault(aligned_addr, PERMISSION, write_op);
        } else if (!pte->dirty && write_op) {
            pte->dirty = 1;
            res = resolve_page_fault(aligned_addr, DIRTY, write_op);
        }

        // return immediately on these fatal faults
        if (res == FAULT_BUS || res == FAULT_SEGV || res == FAULT_NOMEM) {
            printk("Fatal fault of type %u on %s %u", res,
                   write_op ? "write to" : "read from", virt_addr);
            return res;
        }
        // a fault occurred and was resolved, so attempt the operation again
        else if (res != FAULT_NONE) {
            // a more severe fault occurred than ever before on this operation
            if (res > retval) {
                retval = res;
            }
            goto TRY_ACCESS_AGAIN;
        } else {
            pte->accessed = 1;
            buffer += exchange_bytes(virt_addr, aligned_addr, length,
                                     get_system_pointer(pte->base_addr << 12),
                                     buffer, write_op);
        }
    }
    return retval;
}

enum FaultResult nommu_write_to_virtual_address(addr32 virt_addr,
                                                void const *buffer,
                                                uint32_t length) {
    return nommu_access_virtual_address(virt_addr, (void *)buffer, length,
                                        true);
}

enum FaultResult nommu_read_from_virtual_address(addr32 virt_addr, void *buffer,
                                                 uint32_t length) {
    return nommu_access_virtual_address(virt_addr, buffer, length, false);
}

static uint32_t exchange_bytes(addr32 orig_addr, addr32 current_addr,
                               uint32_t length, void *physical_system_pointer,
                               void *user_buffer, bool write_op) {
    addr32 start_addr = MAX(current_addr, orig_addr);
    addr32 end_addr =
        MIN(page_align(start_addr + PAGE_SIZE), orig_addr + length);
    addr32 off = start_addr - current_addr;
    uint32_t difference = end_addr - start_addr;
    if (write_op) {
        memcpy(physical_system_pointer + off, user_buffer, difference);
    } else {
        memcpy(user_buffer, physical_system_pointer + off, difference);
    }
    return difference;
}
