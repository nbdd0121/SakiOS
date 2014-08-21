/**
 * Implement ANSI C's memory operation functions
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/stddef.h"
#include "c/string.h"
#include "c/stdlib.h"

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

/**
 * strdup - duplicate a string
 *
 * @comform SVr4, 4.3BSD, POSIX.1-2001
 */
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *ret = malloc(len);
    memcpy(ret, s, len);
    return ret;
}

int strcmp(const char *s1, const char *s2) {
    for (; *s2 != 0; s1++, s2++) {
        int diff = *s1 - *s2;
        if (diff != 0) {
            return diff;
        }
    }
    return *s1;
}