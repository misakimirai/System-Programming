/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "physical_memory.h"
#include "types.h"
/**
 * The page cache is a hash table used to map filenames and file offsets to
 * corresponding pages in main memory. The page cache is used to share page
 * frames between processes, and is essential in the implementation of the
 * mmap() syscall with MAP_SHARED set to true.
 *
 * In Linux 2.6, the page cache associates inodes and offsets with pages. But
 * for simplicity, we are going to simply associate filenames and offsets with
 * pages instead.
 */

/**
 * Get the page frame associated with a given file and page offset. If no such
 * page frame is located in the cache, this method returns NULL.
 *
 * Note that `offset` must be page-aligned.
 */
struct page *page_cache_get(char const *filename, uint32_t offset);

/**
 * Removes a given `page` struct from the page cache data structure. This
 * amounts to nothing more than a list_unlink operation.
 */
void page_cache_remove(struct page *);

/**
 * Adds a given `page` struct to the page cache. This will be done by
 * obtaining the bucket corresponding to the given `pathname` and
 * `offset`, and inserting `pg` at the head of this linked list.
 */
void page_cache_add(char const *pathname, uint32_t offset, struct page *);

/**
 * Get the head of the linked list corresponding to all struct pages that have
 * the same hash value on their (pathname, offset) combination.
 *
 * Physically, the page cache is just an array of `struct page *` buckets.
 * Empty buckets are simply NULL pointers.
 *
 * Note that the offset must be page-aligned, i.e. a multiple of the page
 * size.
 */
struct page **page_cache_get_bucket(char const *pathname, uint32_t offset);
