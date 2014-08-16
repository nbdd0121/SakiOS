/**
 * Implement ANSI C's memory operation functions
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/stddef.h"
#include "c/string.h"

void *memcpy(void *restrict dest, const void *restrict src, size_t count) {
    size_t i;
    char *d = dest;
    const char *s = src;
    for (i = 0; i < count; i++, d++, s++) {
        *d = *s;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t count) {
    if ((size_t)dest < (size_t)src || (size_t)dest > (size_t)src + count) {
        size_t i;
        char *d = dest;
        const char *s = src;
        for (i = 0; i < count; i++, d++, s++) {
            *d = *s;
        }
    } else {
        size_t i;
        char *d = dest + count;
        const char *s = src + count;
        for (i = 0; i < count; i++, d--, s--) {
            *d = *s;
        }
    }
    return dest;
}

void *memset(void *dest, int val, size_t count) {
    void *ptr = dest;
    for (ptr = dest; count; ptr++, count--) {
        *(char *)ptr = val;
    }
    return dest;
}

size_t strlen(const char *str) {
    size_t counter = 0;
    for (; *str; counter++, str++);
    return counter;
}