/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "types.h"

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x10

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_FAILED (addr32)(-1)

/**
 * The mmap system call creates a virtual memory area (VMA) in the current
 * process' address space (somewhere between the heap and stack) and dedicates
 * this VMA to a given file, setting the appropriate flags in the VMA.
 *
 * Note that no two VMAs in the same process can have overlapping virtual
 * address ranges, and the virtual address range must be exactly `length`
 * bytes large. Also, note that the heap can grow (up) through `syscall_sbrk`,
 * so it's best to allocate new VMAs as close as possible to the stack, whic
 * we assume is fixed.
 *
 * If the `pathname` is not NULL and MAP_ANONYMOUS is not set, the syscall
 * should also obtain a `mapped_file` struct for the pathname
 * (hint: look in `disk.h`), set the VMA's `file` member to that `mapped_file`,
 * and, if MAP_SHARED is set, insert the VMA at the head of the
 * `mapped_file`'s `shared_vma_list`.
 *
 * `length`: size of the newly created VMA. Must be page-aligned.
 *    If the VMA is mapped to a file, note that it is legal to map more pages
 *    than remain in the file, but doing so will result in a bus error when
 *    those pages are accessed.
 * `prot_flags`: can be some combination of PROT_READ, PROT_WRITE, PROT_EXEC,
 *    and PROT_NONE. See `struct vm_area->flags`.
 * `flags`:  must be MAP_SHARED or MAP_PRIVATE, but not both, and possibly
 *    MAP_ANONYMOUS, which indicates that the VMA is not backed by any file.
 *    MAP_SHARED and MAP_ANONYMOUS cannot both be set. See `vm_area->prot_flags`
 * `pathname`: name of the file to be mapped. This is ignored if MAP_ANONYMOUS
 *    is set.
 * `offset`: page-aligned offset in file from which the mapping starts; e.g.
 *    an offset of 4096 means that the virtual address returned maps to the
 *    4096th byte of the file. This is ignored if MAP_ANONYMOUS is set.
 *
 * Returns the page-aligned virtual address of the beginning of the mapped
 * region if the system call succeeds, otherwise returns MAP_FAILED.
 *
 * ERROR HANDLING:
 * -Either MAP_SHARED or MAP_PRIVATE must be provided in flags, but not both
 * -unable to obtain a non-NULL `mapped_file` struct for the pathname when
 *  MAP_ANONYMOUS is not set
 * -Integer arguments must be page aligned
 * -MAP_SHARED and MAP_ANONYMOUS cannot both be set (in the real Linux syscall,
 *  this is not the case)
 */
uint32_t syscall_mmap(uint32_t length, int prot_flags, int flags,
                      char const *pathname, uint32_t offset);

/**
 * If `addr` corresponds to a shared page backed by a memory-mapped file, that
 * page will be synced (written back) to disk if dirty (see disk.h).
 *
 * Returns 0 on success. Returns -1 if `addr` isn't in the current process'
 * virtual address space or if `addr` does not correspond to a shared,
 * memory-mapped file.
 */
int32_t syscall_msync(addr32 addr);

/**
 * Shifts the program break (i.e. the end of the heap) by the given positive
 * offset. The offset must be page-aligned.
 *
 * On success, returns the new program break virtual address. Returns
 * MAP_FAILED on failure.
 */
addr32 syscall_sbrk(uint32_t offset);

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
int32_t syscall_munmap(addr32 addr);
