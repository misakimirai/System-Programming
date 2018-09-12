/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "types.h"
/**
 * This header contains data structures and methods for taking into account
 * virtual memory used by processes and the layout of the process virtual
 * address space.
 */

/**
 * Depicts the virtual address space of a process, as well as a PID. This is
 * based primarily off of the `mm_struct` from the Linux kernel, but is
 * creatively reimagined for your viewing pleasure.
 */
struct process {
    /* process ID */
    uint32_t pid;

    /* sorted, singly linked list of all virtual memory areas */
    struct vm_area *area_list;

    /* global page directory of this process, used by kernel when resolving
     * page faults, and used by the MMU to perform virtual address translation
     */
    struct page_directory *paging_directory;

    struct vm_area *text_segment;
    struct vm_area *data_segment;
    struct vm_area *heap_segment;
    struct vm_area *stack_segment;
};

/**
 * Creates a process. The PID will be automagically set to the first available
 * PID. The text, data, heap, and stack segments will be initialized
 * completely for you.
 */
struct process *spawn_process(void);

/**
 * Kills a process and unmaps all of its VMAs. The process page directory
 * also gets freed for use by the memory manager.
 */
void kill_process(struct process *proc);

/**
 * Virtual memory areas, or VMAs, define contiguous, non-overlapping regions of
 * a process' address space that share the same behavior. Think of the VMA as
 * a "contract" between the process and kernel memory management. The process
 * is legally allowed to access any address in any VMA according to the VMA's
 * permissions, but this memory may or may not be backed by physical pages
 * until the process tries to legally access the address.
 *
 * Each call to `syscall_mmap` will create a new VMA located somewhere
 * between the heap and stack VMAs.
 *
 * If a process tries to access a virtual address not contained in any VMA,
 * the kernel raises a segmentation violation signal (SIGSEGV).
 */
struct vm_area {
    /* pointer to the process address space struct that contains this VMA */
    struct process *proc;

    /* the first virtual address of this virtual memory area */
    uint32_t start_addr;
    /* the ending virtual address of this virtual memory area */
    uint32_t end_addr;

    /* doubly linked list pointers to the next vm_area in the process, and a
     * pointer to the next pointer of the previous vm_area. See
     * `proc->area_list`.
     */
    struct vm_area *area_next;
    struct vm_area **area_prev;

    /* points to resource that depicts memory-mapped file; if this VMA is not
     * mapped to a file, this field shall be NULL
     */
    struct mapped_file *file;
    /* page aligned offset within a memory-mapped file; this value is only
     * meaningful if this VMA corresponds to a memory-mapped file
     */
    uint32_t page_offset;

    /* Linked list pointers that connect together all VMAs from all processes
     * that are mapped to the same file.
     *
     * If this VMA is not used to memory-map a file, these fields shall be NULL.
     */
    struct vm_area *vm_mapped_next;
    struct vm_area **vm_mapped_prev;

    /* Access flags, derived from the `prot_flags` argument in `syscall_mmap`.
     */
    uint32_t prot_read : 1;
    uint32_t prot_write : 1;
    uint32_t prot_exec : 1;

    /* Mapping attribute flags, derived from the `flags` argument in
     * `syscall_mmap`.
     */
    uint32_t map_private : 1;
    uint32_t map_anonymous : 1;
};

/**
 * Allocates a new `struct vm_area` with three of its fields filled out.
 * All of its other fields are zero-initialized and must be filled out
 * manually.
 */
struct vm_area *create_vm_area(struct process *proc, uint32_t start_addr,
                               uint32_t end_addr);

/**
 * Place a vm_area in its sorted position within the `struct process`
 * `area_list` linked list. This method raises an assertion failure if
 * the VMA does not fit in the process virtual address space (when would
 * this happen?).
 */
void sorted_link_vm_area(struct process *proc, struct vm_area *vma);

/**
 * Detach a vm_area from all of its connecting linked lists.
 */
void unlink_vm_area(struct vm_area *vma);

/**
 * Releases the memory associated with a VMA. Note that this does NOT unlink
 * the VMA from all of its associated lists..
 */
void free_vm_area(struct vm_area *vma);

/**
 * Finds the first VMA in the `proc->area_list', after or including `list_node`,
 * that contains the given `virt_addr`. If none is found, then returns NULL.
 *
 * In the degenerate case of a size 0 VMA, this method will return the first
 * such VMA whose `start_addr` field matches the specified virtual address.
 */
struct vm_area *get_containing_vm_area(struct vm_area *list_node,
                                       uint32_t virt_addr);
