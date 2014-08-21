/**
 * Provide assembly functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef ASM_ASM_H
#define ASM_ASM_H

__attribute__((noreturn))
static inline void disableCPU() {
    __asm__  __volatile__("cli;hlt;");
    while (1);
}

#endif
