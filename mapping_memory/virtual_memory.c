/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "kernel.h"
#include "list.h"
#include "physical_memory.h"
#include "syscalls.h"
#include "virtual_memory.h"
#include <stdlib.h>

/**
 * Meant to be used on every VMA already part of a linked list. Ensures that
 * the list is indeed sorted, has nonoverlapping VMAs, and that each VMA's
 * start address does not come before its end address.
 */
#define assert_vma_proper(vma)                                                 \
    assertion(vma->proc, "VMA process struct was NULL: [%08x, %08x]",          \
              vma->start_addr, vma->end_addr);                                 \
    assertion(                                                                 \
        vma->start_addr <= vma->end_addr,                                      \
        "VMA start address after end address: [%08x,%08x) in process %u",      \
        vma->start_addr, vma->end_addr, vma->proc->pid);                       \
    assertion(!vma->area_next || vma->area_next->start_addr >= vma->end_addr,  \
              "VMAs overlap: [%08x,%08x) and [%08x,%08x) in process %u",       \
              vma->start_addr, vma->end_addr, vma->area_next->start_addr,      \
              vma->area_next->end_addr, vma->proc->pid);

static struct process process_table[256];

static struct vm_area *vma_set_flags(struct vm_area *vma, int prot_flags,
                                     int flags) {
    vma->map_anonymous = !!(flags & MAP_ANONYMOUS);
    vma->map_private = !!(flags & MAP_PRIVATE);
    vma->prot_read = !!(prot_flags & PROT_READ);
    vma->prot_write = !!(prot_flags & PROT_WRITE);
    return vma;
}

struct process *spawn_process(void) {
    for (size_t i = 0; i < 256; ++i) {
        if (process_table[i].pid == 0) {
            struct process *p = process_table + i;
            p->pid = i + 1;
            struct page *pg = get_first_unused_page();
            if (!pg) {
                assertion(pg, "No free page frames to spawn process.\n");
                return NULL; // No memory
            }
            const int32_t rd = PROT_READ;
            const int32_t rdwr = PROT_READ | PROT_WRITE;
            const int32_t privanon = MAP_ANONYMOUS | MAP_PRIVATE;
            // text segment is actually memory mapped, but let's not bother
            p->text_segment = create_vm_area(p, 0x08048000, 0x08050000);
            vma_set_flags(p->text_segment, rd, privanon);
            sorted_link_vm_area(p, p->text_segment);

            p->data_segment = create_vm_area(p, 0x08050000, 0x08070000);
            vma_set_flags(p->data_segment, rdwr, privanon);
            sorted_link_vm_area(p, p->data_segment);

            p->heap_segment = create_vm_area(p, 0x08072000, 0x08072000);
            vma_set_flags(p->heap_segment, rdwr, privanon);
            sorted_link_vm_area(p, p->heap_segment);

            p->stack_segment = create_vm_area(p, 0xBFFFE000, 0xC07FE000);
            vma_set_flags(p->stack_segment, rdwr, privanon);
            sorted_link_vm_area(p, p->stack_segment);

            zero_out_page_frame(pg);
            pg->reserved = 1;
            p->paging_directory = get_system_pointer(page_to_phys(pg));
            return p;
        }
    }

    assertion(0, "Spawn failed due to full process table.");
    return NULL;
}

void kill_process(struct process *proc) {
    // switch temporarily to new process context to use ordinary syscall
    bool proc_not_current = proc != current;
    struct process *temp = NULL;

    if (proc_not_current) {
        temp = current;
        context_switch(proc);
    }

    // munmap every VMA
    while (proc->area_list) {
        syscall_munmap(proc->area_list->start_addr);
    }

    // release the page directory
    struct page *pgdir =
        phys_to_page(system_pointer_to_phys(proc->paging_directory));
    pgdir->reserved = 0;

    // switch back to old process context, if there was one
    context_switch(temp);

    proc->pid = 0;
    return;
}

struct vm_area *create_vm_area(struct process *proc, uint32_t start_addr,
                               uint32_t end_addr) {
    struct vm_area *out = calloc(1, sizeof(struct vm_area));
    out->proc = proc;
    out->start_addr = start_addr;
    out->end_addr = end_addr;
    assert_vma_proper(out);
    return out;
}

void free_vm_area(struct vm_area *vma) {
    if (vma->area_next || vma->area_prev || vma->vm_mapped_next ||
        vma->vm_mapped_prev || vma->proc) {
        fprintf(stderr,
                "%s: WARNING: VMA contains dangling pointers. Use "
                "unlink_vm_area() before free to suppress this warning.\n",
                __FUNCTION__);
    }
    free(vma);
}

struct vm_area *get_containing_vm_area(struct vm_area *list_node,
                                       uint32_t virt_addr) {
    list_for_each(list_node, area_next, vma) {
        assert_vma_proper(vma);
        // edge case where VMA address range is empty
        if (vma->end_addr == virt_addr && virt_addr == vma->start_addr) {
            return vma;
        }
        if (vma->end_addr > virt_addr) {
            return vma->start_addr <= virt_addr ? vma : NULL;
        }
    }
    return NULL;
}

void unlink_vm_area(struct vm_area *vma) {
    struct process *proc = vma->proc;
    if (proc) {
        if (vma == proc->text_segment)
            proc->text_segment = NULL;
        else if (vma == proc->data_segment)
            proc->data_segment = NULL;
        else if (vma == proc->heap_segment)
            proc->heap_segment = NULL;
        else if (vma == proc->stack_segment)
            proc->stack_segment = NULL;
    }
    vma->proc = NULL;
    if (vma->area_prev) {
        list_unlink(vma, area_prev, area_next);
    }
    if (vma->vm_mapped_prev) {
        list_unlink(vma, vm_mapped_prev, vm_mapped_next);
    }
}

static bool are_vmas_sorted(struct process *proc) {
    list_for_each(proc->area_list, area_next, vma) {
        if ((vma->area_next && vma->area_next->start_addr < vma->end_addr) ||
            vma->start_addr > vma->end_addr) {
            return false;
        }
    }
    return true;
}

void sorted_link_vm_area(struct process *proc, struct vm_area *target) {
    (void)are_vmas_sorted;
    assert_vma_proper(target);
    if (!proc->area_list) {
        list_link_before(target, &proc->area_list, area_prev, area_next);
        return;
    }
    list_for_each(proc->area_list, area_next, vma) {
        assert_vma_proper(vma);
        assertion(target->end_addr <= vma->start_addr ||
                      target->start_addr >= vma->end_addr,
                  "VMA does not fit in  process address space: [%08x, %08x) in "
                  "process %u",
                  target->start_addr, target->end_addr, proc->pid);
        if (target->end_addr <= vma->start_addr) {
            list_link_before(target, vma->area_prev, area_prev, area_next);
            return;
        }
        if (!vma->area_next) {
            list_link_after(target, vma, area_prev, area_next);
            return;
        }
    }
}
