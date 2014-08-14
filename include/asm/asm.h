/**
 * Provide assembly functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

__attribute__((noreturn))
static inline void disableCPU() {
    __asm__  __volatile__("cli;hlt;");
    while (1);
}
