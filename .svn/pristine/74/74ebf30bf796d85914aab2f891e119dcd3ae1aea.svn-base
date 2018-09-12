/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 *                                                                             *
 *  Begin type forward declarations                                            *
 *                                                                             *
 ******************************************************************************/

#ifndef NULL
#define NULL 0
#endif

/* See page_table.h */

struct page_table;

struct page_directory;

typedef struct page_table_entry pte_t;

typedef struct page_directory_entry pde_t;

union paging_entry;

/* See physical_memory.h */

struct page;

/* See virtual_memory.h */

struct process;

struct vm_area;

/* See disk.h */

struct mapped_file;

/* We are using 32-bit virtual addresses and physical addresses. */
typedef uint32_t addr32;

/*******************************************************************************
 *                                                                             *
 *  Begin size declarations                                                    *
 *                                                                             *
 ******************************************************************************/

/* Size of a page */
#define PAGE_SIZE 4096u

/* Size of a paging entry */
#define ENTRY_SIZE 4u

/* Size of a page frame; this is necessarily the same size as a page, since a
 * page offset is directly mapped to a page frame offset.
 */
#define FRAME_SIZE PAGE_SIZE

/* Number of bits needed to address offset within a given page. Note that this
 * is log2(PAGE_SIZE)
 */
#define NUM_OFFSET_BITS 12u

/* Number of physical pages in RAM. */
#define NUM_PHYSICAL_PAGES 1024u

/* Number of physical bytes in RAM (note 262144 * 4KiB = 1GiB) */
#define PHYSICAL_MEMORY_SIZE (PAGE_SIZE * NUM_PHYSICAL_PAGES)

/* Number of bits in virtual address */
#define VIRTUAL_ADDR_SPACE 32u

/* Number of bits in physical address (in the simulation, very few of these
 * will be used, since we only have 1MiB of physical memory)
 */
#define PHYSICAL_ADDR_SPACE 32u

/* Depth of page table. depth = 1 would mean single page table pointing
 * directly to used physical frames.
 */
#define PAGE_TABLE_DEPTH 2u

/* Number of bits used to represent a single Virtual Page Number (VPN), which
 * is used to index a page table / page directory in order to get an entry.
 * Note that the virtual address is split up into NUM_OFFSET_BITS for the
 * offset, and the rest of the address is split up into
 * PAGE_TABLE_DEPTH equally sized chunks.
 */
#define NUM_VPN_BITS ((VIRTUAL_ADDR_SPACE - NUM_OFFSET_BITS) / PAGE_TABLE_DEPTH)

/* Number of page table entries per page directory / page table. Note that
 * each page directory / page table itself is a page consisting of NUM_ENTRIES
 * entries.
 */
#define NUM_ENTRIES (PAGE_SIZE / ENTRY_SIZE)

/* Size of base physical address */
#define BASE_ADDR_SIZE 20u

/* Logarithm of page cache bucket count */
#define PAGE_HASH_BITS 10u

/* Number of buckets in page cache */
#define PAGE_HASH_SIZE (1ul << PAGE_HASH_BITS)

/*******************************************************************************
 *                                                                             *
 *  Begin helper macros                                                        *
 *                                                                             *
 ******************************************************************************/

/**
 * `typeof` macro evaluates the type of an expression. It behaves similarly
 * to `decltype` from C++ and is useful for writing generic macros. Example:
 *
 * ```
 * typeof(y) x = y + 1;
 * ```
 */
#ifndef typeof
#define typeof __typeof__
#endif

/**
 * Evaluates to minimum of two expressions.
 */
#ifndef MIN
#define MIN(a, b)                                                              \
    ({                                                                         \
        typeof(a) $a = (a), $b = (b);                                          \
        $a < $b ? $a : $b;                                                     \
    })
#endif

/**
 * Evaluates to maximum of two expressions.
 */
#ifndef MAX
#define MAX(a, b)                                                              \
    ({                                                                         \
        typeof(a) $a = (a), $b = (b);                                          \
        $a > $b ? $a : $b;                                                     \
    })
#endif

/**
 * Essentially returns the component of a virtual address that is not aligned
 * to `power_of_two` bits. This is roughly equivalent to
 *
 * `addr % (unsigned)(pow(2, power_of_two)) `
 */
#define misalignment(addr, power_of_two)                                       \
    ((addr) & ((1ul << (power_of_two)) - 1))

/**
 * Zeros out the least significant `power_of_two` bits of `addr`.
 */
#define align(addr, power_of_two)                                              \
    ({                                                                         \
        typeof(power_of_two) $power_of_two = (power_of_two);                   \
        ((addr) >> $power_of_two) << $power_of_two;                            \
    })

#define page_align(addr) align(addr, NUM_OFFSET_BITS)

/**
 * Creates an unsigned bit mask of `width` 1s and the rest 0s. The nonzero
 * bits are the least significant bits.
 */
#define mk_bit_mask(width) ((((uint64_t)1) << (width)) - 1)

/**
 * Computes a power of two.
 */
#define pow2(power_of_two) (((uint64_t)1) << (power_of_two))
