/**
 * Provide memory allocation functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/assert.h"
#include "c-stdlib/malloc.h"
#include "mem-alloc/blockalloc.h"

static allocator_t *allocator = NULL;

void init_allocator(pageman_t *man) {
    allocator = allocator_create(man);
    assert(allocator != NULL);
}

void free(void *addr) {
    allocator_free(allocator, addr);
}

void *malloc(size_t size) {
    void *ret = allocator_malloc(allocator, size);
    assert(ret);
    return ret;
}

void *calloc(size_t nmemb, size_t size) {
    void *ret = allocator_calloc(allocator, nmemb, size);
    assert(ret);
    return ret;
}

void *realloc(void *addr, size_t size) {
    void *ret = allocator_realloc(allocator, addr, size);
    assert(ret);
    return ret;
}

void *aligned_alloc(size_t alignment, size_t size) {
    void *ret = allocator_aligned_alloc(allocator, alignment, size);
    assert(ret);
    return ret;
}
