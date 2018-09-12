/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "assertions.h"
#include "disk.h"
#include "list.h"
#include "page_cache.h"
#include "physical_memory.h"
#include "types.h"
#include <string.h>

#define assert_path_offset_proper(pathname, offset)                            \
    assertion(pathname, "Pathname was NULL!\n");                               \
    assertion(!misalignment(offset, 12), "Offset %08x is not page-aligned!\n", \
              offset);

static struct page *page_cache_buckets[PAGE_HASH_SIZE];

static uint32_t djb2_hash_function(char const *str);

struct page **page_cache_get_bucket(char const *pathname, addr32 offset) {
    assert_path_offset_proper(pathname, offset);
    uint32_t hash_index =
        (djb2_hash_function(pathname) * ((offset >> NUM_OFFSET_BITS) + 1)) %
        PAGE_HASH_SIZE;
    return page_cache_buckets + hash_index;
}

/**
 * Get the page frame associated with a given file and page offset. If no such
 * page frame is located in the cache, this method returns NULL.
 *
 * Note that `offset` must be page-aligned.
 */
struct page *page_cache_get(char const *pathname, addr32 offset) {
    assert_path_offset_proper(pathname, offset);
    struct page **bucket = page_cache_get_bucket(pathname, offset);
    list_for_each(*bucket, hash_next, pg) {
        if (!strcmp(pg->mapping->pathname, pathname) &&
            pg->file_offset == offset) {
            return pg;
        }
    }
    return NULL;
}

/**
 * Removes a given `page` struct from the page cache data structure. This
 * amounts to nothing more than a list_unlink operation.
 *
 * Since the linked list pointers inside of the `page` struct alone are
 * sufficient to remove it from the page cache, you don't even need to
 * compute a hash value to remove it.
 *
 * The appropriate linked list pointers in the `page` struct should both be
 * set to NULL after it is removed from the page cache.
 */
void page_cache_remove(struct page *pg) {
    if (pg->hash_prev || pg->hash_next)
        list_unlink(pg, hash_prev, hash_next);
}

void page_cache_add(char const *pathname, addr32 offset, struct page *pg) {
    struct page **bucket = page_cache_get_bucket(pathname, offset);
    list_link_before(pg, bucket, hash_prev, hash_next);
}

static uint32_t djb2_hash_function(char const *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = (hash << 5) + hash + c;
    }
    return hash;
}
