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
    char *d = dest;
    const char *s = src;
    for (size_t i = 0; i < count; i++, d++, s++) {
        *d = *s;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t count) {
    if ((size_t)dest < (size_t)src || (size_t)dest > (size_t)src + count) {
        char *d = dest;
        const char *s = src;
        for (size_t i = 0; i < count; i++, d++, s++) {
            *d = *s;
        }
    } else {
        char *d = dest + count;
        const char *s = src + count;
        for (size_t i = 0; i < count; i++, d--, s--) {
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

size_t strnlen(const char *str, size_t maxlen) {
    size_t counter = 0;
    for (; *str && counter < maxlen; counter++, str++);
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

char *strndup(const char *s, size_t n) {
    size_t len = strlen(s);
    if (len > n) {
        len = n;
    }
    char *ret = malloc(len + 1);
    memcpy(ret, s, len);
    ret[len] = 0;
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

int memcmp(const void *s1, const void *s2, size_t n) {
    const char *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; i++, p1++, p2++) {
        int diff = *p1 - *p2;
        if (diff != 0) {
            return diff;
        }
    }
    return 0;
}