/**
 * Implement of string.h in ANSI C
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef C_STRING_H
#define C_STRING_H

#include "c/stddef.h"

void *memcpy(void *restrict s1, const void *restrict s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);
char *strdup(const char *s);

int strcmp(const char *s1, const char *s2);

#endif