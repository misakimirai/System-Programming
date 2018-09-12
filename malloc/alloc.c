/**
* Malloc Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct block {
  size_t size;
  struct block *prev;
  struct block *next;
} block;



size_t align(size_t size) {
  return ((size + 7) & ~7);
}


#define BSIZE align(sizeof(block))

static block *head;
static void *start;
static void *end;


void initial() {
  start = sbrk(0);
  head = sbrk(BSIZE);
  head->size = BSIZE | 0b1;
  head->prev = head;
  head->next = head;
  end = sbrk(0);
}



void* findfit(size_t k) {
  block *p = head->next;
  for (p = head->next; p != head && p->size < k; p = p->next);
  if (p != head) {
    return p;
  }
  return NULL;
}



block* split(block* p, size_t size) {
  if (p->size < size + 2 * (BSIZE + sizeof(size_t))) {
    return NULL;
  }
  block* temp1 = (block*)((char*)p + size);
  temp1->size = p->size - size;
  *((size_t*)((char*)temp1 + temp1->size - sizeof(size_t))) = temp1->size;
  return temp1;
}


void coalesce(block *target) {
  int _right = 1;
  int _left = 1;
  block *right = (block*)((char*)target + target->size);
  if ((void*)right >= (void*)end || (right->size & 0b1)) {
    // Out of what we have allocated or it has been used
    _right = 0;
  }
  size_t leftsize = *((size_t*)target - 1);
  block *left;
  if (target == (block*)((char*)start + BSIZE) || (leftsize & 0b1)) {
    // Left is the start or it has been used
    _left = 0;
  }
  else {
    left = (block*)((char*)target - (leftsize & ~1));
  }
  // We now have four cases

  // right and left
  if (_right && _left) {
    // Remove right from the list
    right->prev->next = right->next;
    right->next->prev = right->prev;
    left->size = left->size + target->size + right->size;
    *((size_t*)((char*)left + left->size - sizeof(size_t))) = left->size;
  }

  // only right
  else if (_right) {
    // target will replace right in the list
    target->next = right->next;
    target->prev = right->prev;
    right->next->prev = target;
    right->prev->next = target;
    target->size += right->size;
    *((size_t*)((char*)target + target->size - sizeof(size_t))) = target->size;
  }

  // only left
  else if (_left) {
    // Just change the size
    left->size += target->size;
    *((size_t*)((char*)left + left->size - sizeof(size_t))) = left->size;
  }

  // none
  else {
    target->next = head->next;
    target->prev = head;
    head->next->prev = target;
    head->next = target;
    *((size_t*)((char*)target + target->size - sizeof(size_t))) = target->size;
  }
}



/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void *p = malloc(num * size);
    if (!p) {
      return NULL;
    }
    memset(p, 0, num * size);
    return p;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in align(sizeof(block))bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    if (size == 0) {
      return NULL;
    }
    // If head is NULL then initialize it
    if (!head) {
      initial();
    }
    size_t newsize = BSIZE + align(size) + sizeof(size_t);
    block *p = findfit(newsize);
    if (!p) {
      // p is NULL
      // NO exiting block is fited, build a new onc
      p = sbrk(newsize);
      end = sbrk(0);
      if (*((int*)p) == -1) {
        // sbrk failed
        return NULL;
      }
      p->size = newsize | 0b1;
      // Set the size also at the end
      *((size_t*)((char*)p + BSIZE + align(size))) = p->size;
    }
    else {
      // consider split
      // if (p->size < newsize + 2 * (BSIZE + sizeof(size_t))) {
      //   // Do not split due to size
      //   p->prev->next = p->next;
      //   p->next->prev = p->prev;
      //   p->size |= 0b1;
      //   *((size_t*)((char*)p + p->size - sizeof(size_t))) = p->size;
      //   return (void*) ((char*)p + BSIZE);
      // }
      // // split
      // block* temp1 = (block*)((char*)p + newsize);
      // temp1->size = p->size - newsize;
      // *((size_t*)((char*)temp1 + temp1->size - sizeof(size_t))) = temp1->size;
      // temp1->next = p->next;
      // temp1->prev = p->prev;
      // p->next->prev = temp1;
      // p->prev->next = temp1;
      // p->size = newsize | 0b1;
      // *((size_t*)((char*)p + BSIZE + align(size))) = p->size;
      block *temp1 = split(p, newsize);
      if (!temp1) {
        // Do not split due to size
        p->prev->next = p->next;
        p->next->prev = p->prev;
        *((size_t*)((char*)p + p->size - sizeof(size_t))) = p->size | 0b1;
        // *((size_t*)((char*)p + p->size - sizeof(size_t))) = p->size;
        p->size |= 0b1;
      }
      else {
        temp1->next = p->next;
        temp1->prev = p->prev;
        p->next->prev = temp1;
        p->prev->next = temp1;
        p->size = newsize | 0b1;
        *((size_t*)((char*)p + BSIZE + align(size))) = p->size;
        // *((size_t*)((char*)p + BSIZE + align(size))) = newsize;
      }
    }
    return (void*) ((char*)p + BSIZE);
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
 
void free(void *ptr) {
    // implement free!
    if(ptr == NULL){
      return;
    }
    block* target = (block*)((char*)ptr - BSIZE);
    target->size = target->size & ~1;
    coalesce(target);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (!ptr) {
      return malloc(size);
    }
    if (size == 0) {
      free(ptr);
      return NULL;
    }
    block *temp = (block*) ((char*)ptr - BSIZE);
    size_t oldsize = (temp->size & ~1) - BSIZE - sizeof(size_t);
    size_t copysize = oldsize > size ? size : oldsize;
    void *newptr = malloc(size);
    if (newptr) {
      memcpy(newptr, ptr, copysize);
    }
    else {
      // malloc failed
      return NULL;
    }
    free(ptr);
    return newptr;
}
