/**
 * Provide bitmap functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef DATA_STRUCT_BITMAP_H
#define DATA_STRUCT_BITMAP_H

#include "c/stddef.h"
#include "c/stdbool.h"

/**
 * bitmap
 * The bitmap is defined as unsigned char, so bitmap* can points to unsigned char*,
 * which is the first element actually in the bitmap.
 */
typedef unsigned char bitmap_t;

static inline bool bitmap_switch(bitmap_t *b, size_t index) {
    unsigned char mask = (unsigned char)(1 << (index & 7));
    return !((b[index >> 3] ^= mask)&mask);
}

static inline bool bitmap_clear(bitmap_t *b, size_t index) {
    unsigned char mask = (unsigned char)(1 << (index & 7));
    bool ret = b[index >> 3] & mask;
    b[index >> 3] &= ~mask;
    return !!ret;
}

static inline bool bitmap_set(bitmap_t *b, size_t index) {
    unsigned char mask = (unsigned char)(1 << (index & 7));
    bool ret = b[index >> 3] & mask;
    b[index >> 3] |= mask;
    return !!ret;
}

#endif
