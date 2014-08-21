/**
 * Provide memory allocation functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/assert.h"
#include "c-stdlib/malloc.h"
#include "mem-alloc/blockalloc.h"

static allocator_t *allocator;

void init_allocator(pageman_t *man) {
    allocator = allocator_create(man);
    assert(allocator != NULL);
}

void free(void *addr) {
    allocator_free(allocator, addr);
}

void *malloc(size_t size) {
    return allocator_malloc(allocator, size);
}

void *calloc(size_t nmemb, size_t size) {
    return allocator_calloc(allocator, nmemb, size);
}

void *realloc(void *addr, size_t size) {
    return allocator_realloc(allocator, addr, size);
}

void *aligned_alloc(size_t alignment, size_t size) {
    return allocator_aligned_alloc(allocator, alignment, size);
}
