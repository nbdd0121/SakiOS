/**
 * Header file providing utility for log2
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stddef.h"

static inline size_t log2(size_t num) {
    size_t ret;
    __asm__ __volatile__("bsr %1, %0":"=r"(ret):"r"(num));
    return ret;
}
