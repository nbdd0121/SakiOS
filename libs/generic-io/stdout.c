/**
 * Implement some platform independent stdio functions
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/stddef.h"
#include "c/stdio.h"

int puts(const char *str) {
    int count = 0;
    for (; *str; str++, count++) {
        putchar(*str);
    }
    return count;
}
