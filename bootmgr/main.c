#include <stdint.h>

/**
 * C code entrance.
 * Notice that void main(void) is not violence of C standard, because this is not in a hosted environment.
 */
void main(void) {
    uint8_t *video = (uint8_t *)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        *(video++) = ' ';
        *(video++) = 0x0F;
    }
}