/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "disk.h"
#include "physical_memory.h"
#include "types.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static uint32_t swap_bitmap[NUM_PHYSICAL_PAGES >> 5];

static char const *swap_filename = "./tmp/swapfile";
static char *mmapped_swap;
static struct mapped_file mapped_file_array[1024];

// computes index / 32, 1ul << index % 32 to the right bit slot and mask
#define unpack_index(index, bit_block, bit_mask)                               \
    uint32_t bit_block = (index) >> 5;                                         \
    uint32_t bit_mask = 1ul << (((index) - (bit_block << 5)));

#define assert_index_proper(index)                                             \
    assertion((index) < NUM_PHYSICAL_PAGES, "Invalid swap index %08x!",        \
              (index));

#define assert_page_offset_proper(pg)                                          \
    assertion(!misalignment(pg->file_offset, 12),                              \
              "Struct page file offset %08x is misaligned!\n",                 \
              pg->file_offset);

struct mapped_file *obtain_mapped_file(char const *pathname) {
    assertion(pathname, "Pathname was NULL!");
    char *true_path = realpath(pathname, NULL);
    // File does not exist
    if (!true_path) {
        return NULL;
    }
    for (size_t i = 0; i < 1024; ++i) {
        if (mapped_file_array[i].pathname &&
            !strcmp(true_path, mapped_file_array[i].pathname)) {
            free(true_path);
            return mapped_file_array + i;
        }
    }
    for (size_t i = 0; i < 1024; ++i) {
        if (!mapped_file_array[i].reference_count) {
            free(mapped_file_array[i].pathname);
            mapped_file_array[i].pathname = true_path;
            assertion(
                mapped_file_array[i].shared_vma_list == NULL,
                "Reference count of mapped_file hit 0, but `shared_vma_list` "
                "is not empty.");
            return mapped_file_array + i;
        }
    }
    assertion(0, "No more mapped files can be created! This shouldn't happen.");
    return NULL;
}

/**
 * Tries to open a file descriptor for reading or writing and sets the file
 * descriptor offset to the amount spacified by `pg->mapping->offset`.
 */
static int try_open_with_offset(struct page *pg, bool write_op);

/**
 * Returns the index of a free swap slot. If there are no more free swap
 * slots, this functions returns `(uint32_t) -1`.
 */
uint32_t get_free_swap_index(void) {
    for (uint32_t i = 0; i < NUM_PHYSICAL_PAGES / 32; ++i) {
        if (swap_bitmap[i] != UINT32_MAX) {
            for (uint32_t j = 0; j < 32; ++j) {
                if (!(swap_bitmap[i] & (1 << j))) {
                    return (i << 5) + j;
                }
            }
        }
    }
    return (uint32_t)-1;
}

/**
 * Marks the swap slot at the given `index` as unused. This should be used
 * whenever a private page gets unmapped while swapped out.
 */
int free_swap_slot(uint32_t index) {
    assert_index_proper(index);
    unpack_index(index, bit_block, bit_mask);
    // was already freed
    if (!(swap_bitmap[bit_block] & bit_mask)) {
        return -1;
    }
    swap_bitmap[bit_block] &= ~bit_mask;
    return 0;
}

/**
 * Writes the page frame of physical memory represented by the given
 * `struct page` to the given swap index. This will also mark the swap slot as
 * used. However, it does nothing else.
 *
 * If the given swap slot is already being used, returns -1. Returns 0
 * otherwise.
 */
int write_page_to_swap(uint32_t index, struct page *pg) {
    assert_index_proper(index);
    unpack_index(index, bit_block, bit_mask);
    if (swap_bitmap[bit_block] & bit_mask) {
        return -1; // swap slot already in use
    }
    // mark swap slot as used
    swap_bitmap[bit_block] |= bit_mask;
    addr32 phys_addr_start = page_to_phys(pg);
    memcpy(mmapped_swap + (index << 12), get_system_pointer(phys_addr_start),
           4096);
    usleep(3000);
    return 0;
}

/**
 * Reads the contents of swap stored at the indexed swap slot into the physical
 * page frame represented by the given `struct page`. This will also mark the
 * swap slot as unused. However, it does nothing else.
 *
 * If the given swap slot is not in use, returns -1. Returns 0 otherwise.
 */
int read_page_from_swap(uint32_t index, struct page *pg) {
    assert_index_proper(index);
    addr32 phys_addr_start = page_to_phys(pg);
    unpack_index(index, bit_block, bit_mask);
    if (!(swap_bitmap[bit_block] & bit_mask)) {
        return -1; // swap slot not in use
    }
    swap_bitmap[bit_block] &= ~bit_mask;
    memcpy(get_system_pointer(phys_addr_start), mmapped_swap + (index << 12),
           4096);
    usleep(3000);
    return 0;
}

static int try_open_with_offset(struct page *pg, bool write_op) {
    if (!pg->mapping || !pg->mapping->pathname) {
        return -1;
    }
    int fd = open(pg->mapping->pathname, write_op ? O_WRONLY : O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    if (lseek(fd, 0, SEEK_END) <= pg->file_offset) {
        goto TRY_OPEN_FAIL;
    }
    if (lseek(fd, pg->file_offset, SEEK_SET) == (off_t)-1) {
        goto TRY_OPEN_FAIL;
    }
    return fd;
TRY_OPEN_FAIL:
    close(fd);
    return -1;
}

/**
 * Given a `struct page` representing a physical frame that backs a
 * memory-mapped  file, this function will read however many bytes are left in
 * the file or the entire page frame size, whichever is smaller, into that
 * page frame.
 *
 * Note that this requires the `mapping` and `offset` components of the
 * `struct page` to be set properly.
 *
 * Returns -1 if this operation fails. The page fault handler should respond
 * to this event by returning a bus error.
 */
int read_page_from_disk(struct page *pg) {
    assert_page_offset_proper(pg);
    int fd = try_open_with_offset(pg, false);
    if (fd == -1) {
        return -1;
    }
    zero_out_page_frame(pg);
    char kernel_buffer[4096] = {0};
    ssize_t read_count = read(fd, kernel_buffer, 4096);
    close(fd);
    if (read_count <= 0) {
        return -1;
    }
    size_t phys_addr_start = page_to_phys(pg);
    memcpy(get_system_pointer(phys_addr_start), kernel_buffer, read_count);
    usleep(3000);
    return 0;
}

/**
 * Given a `page` representing a physical frame that backs a memory-mapped
 * file, this function will write however many bytes are left in the file or
 * the entire page frame size, whichever is smaller, into that file.
 *
 * Note that this requires the `mapping` and `offset` components of the
 * `struct page` to be set properly.
 *
 * Returns -1 if this operation fails. The page fault handler should respond
 * to this event by returning a bus error.
 */
int write_page_to_disk(struct page *pg) {
    assert_page_offset_proper(pg);
    int fd = try_open_with_offset(pg, true);
    if (fd == -1) {
        return -1;
    }
    off_t index = lseek(fd, 0, SEEK_CUR);
    off_t write_count = lseek(fd, 0, SEEK_END) - index;
    lseek(fd, index, SEEK_SET);
    if (write_count > 4096) {
        write_count = 4096;
    }

    addr32 phys_addr_start = page_to_phys(pg);
    ssize_t write_val =
        write(fd, get_system_pointer(phys_addr_start), write_count);
    close(fd);
    if (write_val < 0) {
        return -1;
    }
    usleep(3000);
    return 0;
}

/**
 * Initialize swap file at program initialization. Ironically, this uses
 * MMAP.
 */
static void __attribute__((constructor)) initialize_swap(void) {
    int swap_fd =
        open(swap_filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (swap_fd < 0) {
        fprintf(stderr, "Failed to create swapfile! Exiting...\n");
        exit(1);
    }
    if (ftruncate(swap_fd, PHYSICAL_MEMORY_SIZE) < 0) {
        fprintf(stderr, "Failed to resize swapfile! Exiting...\n");
        exit(1);
    }
    mmapped_swap = mmap(NULL, PHYSICAL_MEMORY_SIZE, PROT_READ | PROT_WRITE,
                        MAP_SHARED, swap_fd, 0);
    close(swap_fd);
}
