/**
 * C code entrance of boot manager.
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdio.h"
#include "mem-alloc/pageman.h"

/**
 * C code entrance.
 * Notice that void main(void) is not violence of C standard,
 * because this is not in a hosted environment.
 */
void main(void) {
    /* Clear the screen */
    putchar('\f');

    puts("Welcome Home!");

    pageman_t *man = pageman_create(NULL, 0x100000, (void *)0x100000, 0x100);
    for (int i = 0; i < 25; i++) {
        printf("\n%p", pageman_alloc(man, 0));
    }

}

