/**
 * Provide assembly functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef ASM_ASM_H
#define ASM_ASM_H

#include "c/stdint.h"
#include "c/stddef.h"

__attribute__((noreturn))
static inline void disableCPU() {
    while (1)
        __asm__  __volatile__("cli;hlt;");
}

/**
 * invoke inb assembly instruction
 * @param port  the port to read from
 * @return      the readed value
 */
static inline uint8_t readPort8(uint16_t port) {
    uint8_t val;
    __asm__  __volatile__("inb %1, %0":"=a"(val):"Nd"(port));
    return val;
}

/**
 * invoke outb assembly instruction
 * @param port  the port to write to
 * @param val   the value which will be written into the port
 */
static inline void writePort8(uint16_t port, uint8_t val) {
    __asm__  __volatile__("outb %0, %1"::"a"(val), "Nd"(port));
}

static inline void repReadPort16(uint16_t port, void *buffer, size_t count) {
    __asm__  __volatile__(
        "cld;"
        "rep insw"
        ::"d"(port), "D"(buffer), "c"(count>>1):"memory", "cc");
}

static inline void repWritePort16(uint16_t port, void *buffer, size_t count) {
    __asm__  __volatile__(
        "cld;"
        "rep outsw"
        ::"d"(port), "S"(buffer), "c"(count>>1):"memory", "cc");
}

#endif
