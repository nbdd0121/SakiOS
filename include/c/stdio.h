/**
 * Implement of stdio.h in ANSI C
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef C_STDIO_H
#define C_STDIO_H

#include "c/stddef.h"

int putchar(int c);
int puts(const char *s);

int printf(const char *fmt, ...);

#endif