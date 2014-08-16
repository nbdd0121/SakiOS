/**
 * Provide pseduo random number generator
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdlib.h"

static int seed;

int rand(void) {
    seed = ((seed * 1103515245) + 12345) & 0x7fffffff;
    return seed;
}

void srand(unsigned int s) {
    seed = s;
}
