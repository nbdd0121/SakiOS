/**
 * Provide memory block allocation mechanism
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/string.h"
#include "c/stdbool.h"
#include "c/assert.h"

#include "mem-alloc/pageman.h"
#include "mem-alloc/blockalloc.h"

#include "data-struct/list.h"

#include "util/alignment.h"
#include "util/log2.h"

enum {
    PAGE_SIZE = 4096,
    BLOCK_SIZE = sizeof(size_t) * 2,
    MAX_SIZE = PAGE_SIZE - BLOCK_SIZE * 2,
    ARR_LEN = MAX_SIZE / BLOCK_SIZE
};

struct struct_allocator_t {
    pageman_t *man;
    list_t blocks[ARR_LEN];
};

typedef struct struct_block_t {
    struct struct_block_t *prev;
    size_t size;
    list_t list;
} block_t;

static size_t getPagePow(size_t size) {
    size_t pageNum = alignTo(size, PAGE_SIZE) / PAGE_SIZE;
    size_t numLog2 = log2(pageNum);
    if (1 << numLog2 == pageNum) {
        return numLog2;
    } else {
        return numLog2 + 1;
    }
}

static inline block_t *nextBlock(block_t *this) {
    return (block_t *)((size_t)&this->list + (this->size & ~3));
}

static inline block_t *prevBlock(block_t *this) {
    return this->prev;
}

static inline void markFree(allocator_t *al, block_t *b) {
    list_addFirst(&al->blocks[b->size / BLOCK_SIZE - 1], &b->list);
    b->size &= ~1;
}

static inline void markUsed(block_t *b) {
    list_remove(&b->list);
    b->size |= 1;
}

static inline bool isFree(block_t *b) {
    return !(b->size & 1);
}

static inline void splitBlock(block_t *first, size_t size) {
    block_t *third = nextBlock(first);
    first->size = size | 1;
    block_t *second = nextBlock(first);
    second->prev = first;
    third->prev = second;
    second->size = (size_t)third - (size_t)&second->list + 1;
    assert(nextBlock(first) == second);//TODO
}

/* Merge two *Used* block */
static void mergeBlock(block_t *first) {
    block_t *second = nextBlock(first);
    block_t *third = nextBlock(second);
    first->size += second->size + offsetof(block_t, list) - 1;
    third->prev = first;
    assert(nextBlock(first) == third);
}

allocator_t *allocator_create(pageman_t *man) {
    allocator_t *allocator = pageman_alloc(man, getPagePow(sizeof(allocator_t)));
    assert(allocator);
    allocator->man = man;
    for (int i = 0; i < ARR_LEN; i++) {
        list_empty(&allocator->blocks[i]);
    }
    return allocator;
}

void allocator_free(allocator_t *al, void *addr) {
    /* According to c spec, we do nothing if null */
    if (addr == NULL) {
        return;
    }

    block_t *block = GET_DATA(addr, block_t, list);

    if (block->size & 4) {
        block = block->prev;
    }

    /* If it is a big block and is directly allocated by pageman */
    if (block->size & 2) {
        pageman_free(al->man, block, getPagePow(sizeof(block_t) + block->size));
        return;
    }

    block_t *prev = prevBlock(block);
    block_t *next = nextBlock(block);

    /* We are the first block, and the next block is the protector:
     * we merged successfully into one page. */
    if (prev == NULL && next->size == 1) {
        pageman_free(al->man, block, 0);
        return;
    }
    /* If we can merge to previous block */
    if (prev != NULL && isFree(prev)) {
        markUsed(prev);
        mergeBlock(prev);
        allocator_free(al, &prev->list);
        return;
    }
    /* If we can merge to next block */
    if (isFree(next)) {
        markUsed(next);
        mergeBlock(block);
        allocator_free(al, &block->list);
        return;
    }
    markFree(al, block);
}

void *allocator_malloc(allocator_t *al, size_t size) {
    if (size > MAX_SIZE) {
        block_t *block = pageman_alloc(al->man, getPagePow(sizeof(block_t) + size));
        block->size = size | 3;
        return &block->list;
    }

    if (size < BLOCK_SIZE) {
        size = BLOCK_SIZE;
    } else {
        size = alignTo(size, BLOCK_SIZE);
    }

    int id = size / BLOCK_SIZE - 1;
    list_t *list = &al->blocks[id];
    /* If there is a block of the same size */
    if (!list_isEmpty(list)) {
        /* We appreciate that and return */
        block_t *block = GET_DATA(list->next, block_t, list);
        markUsed(block);
        return &block->list;
    }


    list = &al->blocks[++id];
    /* Next block size. If we seperate this block,
     * we will get a block of size 0, so, it is better
     * to give a free upgrade to the requester */
    if (!list_isEmpty(list)) {
        block_t *block = GET_DATA(list->next, block_t, list);
        markUsed(block);
        return &block->list;
    }

    for (id++; id < ARR_LEN; id++) {
        list_t *list = &al->blocks[id];
        if (!list_isEmpty(list)) {
            block_t *block = GET_DATA(list->next, block_t, list);
            markUsed(block);
            splitBlock(block, size);
            markFree(al, nextBlock(block));
            return &block->list;
        }
    }

    /* If we run of out memory */
    void *page = pageman_alloc(al->man, 0);
    if (page == NULL) {
        return NULL;
    }

    block_t *firstBlock = page;
    firstBlock->prev = NULL;
    firstBlock->size = size | 1;

    block_t *rest = nextBlock(firstBlock);
    rest->prev = firstBlock;
    rest->size = (size_t)page + PAGE_SIZE - (size_t)&rest->list - offsetof(block_t, list);

    block_t *endProtect = nextBlock(rest);
    endProtect->prev = rest;
    endProtect->size = 1;

    markFree(al, rest);
    return &firstBlock->list;
}

void *allocator_calloc(allocator_t *al, size_t nmemb, size_t size) {
    void *mem = allocator_malloc(al, nmemb * size);
    if (mem == NULL) {
        return NULL;
    }
    memset(mem, 0, nmemb * size);
    return mem;
}

void *allocator_realloc(allocator_t *al, void *addr, size_t size) {
    size_t original = GET_DATA(addr, block_t, list)->size;
    void *ret = allocator_malloc(al, size);
    if (ret == NULL) {
        return NULL;
    }
    memcpy(ret, addr, original > size ? size : original);
    allocator_free(al, addr);
    return ret;
}

void *allocator_aligned_alloc(allocator_t *al, size_t alignment, size_t size) {
    if ((1 << log2(alignment)) != alignment) {
        return NULL;
    }
    if (alignment <= sizeof(size_t) * 2) {
        return allocator_malloc(al, size);
    }

    size_t aligned_size = size + alignment - sizeof(size_t) * 2;
    void *ret = allocator_malloc(al, aligned_size);
    void *alignedRet = (void *)alignTo((size_t)ret, alignment);

    block_t *block = GET_DATA(alignedRet, block_t, list);
    block->size = 4;
    block->prev = GET_DATA(ret, block_t, list);

    return alignedRet;
}
