/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "kernel.h"
#include "nommu.h"
#include "page_table.h"

/**
 * Performs either a write from `buffer` to `virt_addr` of `length` bytes
 * when `write_op == 1`, or a read from `virt_addr` to `buffer` of `length`
 * bytes when `write_op == 0`.
 *
 * This relies on the value of the `current` global variable in order to select
 * a process page table over which to perform virtual address translation.
 */
enum FaultResult nommu_access_virtual_address(addr32 virt_addr, void *buffer,
                                              uint32_t length, bool write_op);

/**
 * Wrapper function for `nommu_access_virtual_address` where `write_op == 1`.
 */
enum FaultResult nommu_write_to_virtual_address(addr32 virt_addr,
                                                void const *buffer,
                                                uint32_t length);

/**
 * Wrapper function for `nommu_access_virtual_address` where `write_op == 0`.
 */
enum FaultResult nommu_read_from_virtual_address(addr32 virt_addr, void *buffer,
                                                 uint32_t length);
/**
 * Macros for more human-readable reads. This should be roughly equivalent to
 * `variable = ((typeof(variable) *)addr)[index];`
 */
#define nommu_read(variable, addr, index)                                      \
    (nommu_read_from_virtual_address(addr + sizeof(variable) * index,          \
                                     &(variable), sizeof(variable)))

/**
 * Macros for more human-readable writes. This should be roughly equivalent to
 * `((typeof(variable) *)addr)[index] = variable;`
 */
#define nommu_write(variable, addr, index)                                     \
    (nommu_write_to_virtual_address(addr + sizeof(variable) * index,           \
                                    &(variable), sizeof(variable)))
