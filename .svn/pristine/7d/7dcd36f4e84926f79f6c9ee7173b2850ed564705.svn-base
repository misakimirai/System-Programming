/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "page_table.h"
#include "types.h"

/**
 * Current process address space.
 */
extern struct process *current;

/**
 * Describes why a page fault occurred, according to the MMU. The kernel has
 * to make use of this information in order to perform the right operations in
 * software. NOT_PRESENT involves setting up page tables and/or page frames
 * and/or retrieving something from the disk. DIRTY simply involves changing
 * some flags on `struct page`s. PERMISSION implies a probable segfault
 * (in the real kernel it may also imply the need for a copy-on-write
 * operation).
 *
 * Real CPUs do write the cause of a page fault exception to a register before
 * calling the kernel page fault handler. The x86 does not, however, produce
 * a page fault when setting the dirty flag. The simulation does for
 * simplicity.
 */
enum FaultReason {
    /* PTE does not exist or was marked not-present */
    NOT_PRESENT,
    /* PTE was not dirty at first, but process performed a write to the page */
    DIRTY,
    /* PTE was marked read-only but was written to, or user-supervisor violation
     */
    PERMISSION
};

/**
 * The page fault handler describes the result of a page fault by returning
 * one of these flags. This is not realistic and serves mostly for grading
 * and instructional purposes.
 */
enum FaultResult {
    /* No fault at all */
    FAULT_NONE,
    /* Minor page fault resolved (no need to load anything from disk/swap)*/
    FAULT_MINOR,
    /* Major page fault resolved (something was loaded from/to disk/swap) */
    FAULT_MAJOR,
    /* Bus error; page failed to load from/to disk (see disk.h) */
    FAULT_BUS,
    /* No memory left, cannot swap out any pages */
    FAULT_NOMEM,
    /* Segmentation violation */
    FAULT_SEGV
};

/**
 * Simply switches to the new process.
 */
void context_switch(struct process *);

/**
 * Resolves a page fault. The reasoning behind this is rather complicated,
 * so it's probably easier to read through the code and its comments.
 */
enum FaultResult resolve_page_fault(uint32_t virtual_address,
                                    enum FaultReason why, bool write_op);
