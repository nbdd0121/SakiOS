/**
 * Header file providing utility for alignment
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef UTIL_ALIGNMENT_H
#define UTIL_ALIGNMENT_H

#include "c/stddef.h"

static inline size_t alignTo(size_t val, size_t alignment) {
    return ((val - 1) / alignment + 1) * alignment;
}

static inline size_t alignDown(size_t val, size_t alignment) {
    return val / alignment * alignment;
}

#endif