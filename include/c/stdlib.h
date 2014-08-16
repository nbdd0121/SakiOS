/**
 * Implement of stdlib.h in ANSI C.
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef C_STDLIB_H
#define C_STDLIB_H

#include "c/stddef.h"

#define RAND_MAX 0x7fffffff

int rand(void);
void srand(unsigned int seed);

#endif
