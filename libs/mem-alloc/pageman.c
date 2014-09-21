/**
 * Provide page allocation mechanism
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "data-struct/bitmap.h"
#include "data-struct/list.h"

#include "c/string.h"
#include "c/assert.h"
#include "mem-alloc/pageman.h"
#include "util/alignment.h"

enum {
    PAGE_SIZE = 4096,
    TOTAL_LEVEL = 18,
};

struct struct_pageman_t {
    bitmap_t *bitmaps[TOTAL_LEVEL];
    list_t lists[TOTAL_LEVEL];
    void *base;
    size_t limit;
    size_t spare;
};

static inline size_t getOffset(void *base, void *addr, size_t level) {
    size_t off = (size_t)addr - (size_t)base;
    off /= PAGE_SIZE;
    return off >> (level + 1);
}

static inline void *getBuddy(void *base, void *addr, size_t level) {
    size_t off = (size_t)addr - (size_t)base;
    return (void *)((off ^ ((size_t)PAGE_SIZE << level)) + (size_t)base);
}

static inline void *getSuper(void *base, void *addr, size_t level) {
    size_t off = (size_t)addr - (size_t)base;
    return (void *)((off & ~((size_t)PAGE_SIZE << level)) + (size_t)base);
}

/**
 * pageman_create
 * Create an instance of page manager using buddy algorithm.
 *
 * @param base      The base of all memories
 * @param limit     The maximum pages possible to allocate
 * @param firstAval The base of first avaliable memory block
 * @param firstSize The size (measured in pages) of first block avaliable
 * @return          If "firstSize" is not big enough to store the structure, NULL will be returned.
 *                  Otherwise the pointer to the manager will be returned.
 */
pageman_t *pageman_create(void *base, size_t limit, void *firstAval, size_t firstSize) {
    size_t totalSize = 0;
    size_t levelSize, level;
    void *lastPage = (char *)base + (limit - 1) * PAGE_SIZE;
    /* We need to fill in this array first before fill in the real one */
    bitmap_t *bitmaps[TOTAL_LEVEL];
    for (level = 0; level < TOTAL_LEVEL; level++) {
        /* The first byte in the bitmap is the next byte after all previous bitmaps */
        bitmaps[level] = (unsigned char *)firstAval + totalSize + sizeof(pageman_t);
        /* Get the number of bit occupied by a certain level
         * The size of the bitmap is the index of the last bit plus one.
         * And don't forget that we will count the size in bytes instead of bits
         */
        levelSize = alignTo(getOffset(base, lastPage, level) + 1, 8) / 8;
        totalSize += levelSize;
    }
    /* Calculate the size of pageman_t structure */
    size_t pageCost = alignTo(totalSize + sizeof(pageman_t), PAGE_SIZE) / PAGE_SIZE;
    /* We need this argument basicly to check if condtion is satisfied */
    assert(pageCost <= firstSize);
    /* Initialize the sturcture */
    pageman_t *man = firstAval;
    man->base = base;
    man->limit = limit;
    man->spare = 0;
    memcpy(man->bitmaps, bitmaps, sizeof(bitmaps));
    memset((unsigned char *)firstAval + sizeof(pageman_t), 0, totalSize);//TODO NEED?NEED.
    for (level = 0; level < TOTAL_LEVEL; level++)
        list_empty(man->lists + level);
    /* Since only the manager knows how much memory it used, we need
     * to free the first block inside this function */
    pageman_freeBlock(man, (void *)((size_t)firstAval + (size_t)PAGE_SIZE * pageCost), (firstSize - pageCost)*PAGE_SIZE);

    return man;
}

/**
 * pageman_free
 * Return a piece of memory back to the manager.
 *
 * @param bpm       The pointer to manager instance
 * @param addr      The start of the block
 * @param size      Size of the block, meaning that PAGE_SIZE<<size is spare.
 */
void pageman_free(pageman_t *bpm, void *addr, size_t size) {
    size_t offset = getOffset(bpm->base, addr, size);
    /* If the previous value was true, a combination will take place. */
    if (size < TOTAL_LEVEL && bitmap_switch(bpm->bitmaps[size], offset)) {
        bpm->spare -= PAGE_SIZE << size;
        /* If its buddy is free, we remove the buddy page from the bool. */
        list_remove((list_t *)getBuddy(bpm->base, addr, size));
        /* Then we free the combined one */
        pageman_free(bpm, getSuper(bpm->base, addr, size), size + 1);
    } else {
        bpm->spare += PAGE_SIZE << size;
        /* If its buddy is busy, we just return the page into the pool */
        list_addFirst(bpm->lists + size, (list_t *)addr);
    }
}

/**
 * pageman_alloc
 * Allocate a piece of memory from the manager.
 *
 * @param bpm       The pointer to manager instance
 * @param size      Size of the block, meaning that PAGE_SIZE<<size should be allocated.
 * @return          The start of the block
 */
void *pageman_alloc(pageman_t *bpm, size_t size) {
    /* We don't have ability to cover more than TOTAL_LEVEL */
    if (size >= TOTAL_LEVEL) {
        return NULL;
    }
    if (list_isEmpty(bpm->lists + size)) {
        /* Get a super one */
        void *addr = pageman_alloc(bpm, size + 1);
        /* Assurance to prevent out of memory */
        if (addr == NULL) {
            return NULL;
        }
        /* Slice it ^_^ */
        pageman_free(bpm, getBuddy(bpm->base, addr, size), size);
        return addr;
    } else {
        bpm->spare -= PAGE_SIZE << size;
        /* Get the first one out from the linked list */
        list_t *first = bpm->lists[size].next;
        /* Remove the block from the spare list */
        list_remove(first);
        /* Set the bit in the bitmap */
        bitmap_switch(bpm->bitmaps[size], getOffset(bpm->base, first, size));
        return first;
    }
}

/**
 * pageman_freeBlock
 * Return a piece of memory back to the manager.
 *
 * @param bpm       The pointer to manager instance
 * @param addr      The start of the block
 * @param size      Size of the block measured in bytes
 */
void pageman_freeBlock(pageman_t *bpm, void *addr, size_t size) {
    /* Maually align, note during the align process, some memory will never be allocated */
    size_t off = alignTo((size_t)addr, PAGE_SIZE) - (size_t)bpm->base;
    size = alignDown(size, PAGE_SIZE);

    /* These two following loops just slice the memory into aligned pieces and free them */
    size_t i, single_size;
    for (i = 0; i <= TOTAL_LEVEL; i++) {
        single_size = (size_t)PAGE_SIZE << i;
        if ((off & single_size) && (size >= single_size)) {
            pageman_free(bpm, (void *)((size_t)bpm->base + off), i);
            off += single_size;
            size -= single_size;
        }
    }

    single_size = (size_t)PAGE_SIZE << TOTAL_LEVEL;
    while (size >= single_size) {
        pageman_free(bpm, (void *)((size_t)bpm->base + off), i);
        off += single_size;
        size -= single_size;
    }

    for (i = TOTAL_LEVEL - 1; i != (size_t) - 1; i--) {
        single_size = (size_t)PAGE_SIZE << i;
        if (size >= single_size) {
            pageman_free(bpm, (void *)((size_t)bpm->base + off), i);
            off += single_size;
            size -= single_size;
        }
    }
}

/**
 * pageman_spare
 * Query spare size in a given pageman_t instance.
 *
 * @param bpm       The pointer to manager instance
 * @return          The memory available in page manager measured in bytes
 */
size_t pageman_spare(pageman_t *bpm) {
    return bpm->spare;
}
