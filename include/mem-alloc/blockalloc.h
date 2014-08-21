/**
 * Header file for block allocator
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef MEM_ALLOC_BLOCKALLOC_H
#define MEM_ALLOC_BLOCKALLOC_H

#include "c/stddef.h"
#include "mem-alloc/pageman.h"

typedef struct struct_allocator_t allocator_t;
allocator_t *allocator_create(pageman_t *man);
void allocator_free(allocator_t *al, void *addr);
void *allocator_malloc(allocator_t *al, size_t size);
void *allocator_calloc(allocator_t *al, size_t nmemb, size_t size);
void *allocator_realloc(allocator_t *al, void *addr, size_t size);
void *allocator_aligned_alloc(allocator_t *al, size_t alignment, size_t size);

#endif