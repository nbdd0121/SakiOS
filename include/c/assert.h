/**
 * Implement of assert.h in ANSI C.
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef C_ASSERT_H
#define C_ASSERT_H

#ifdef NDEBUG

#define assert(ignore) ((void)0)

#else

#include "asm/asm.h"
#include "c/stdio.h"

#define assert(expr) do{\
        if(!(expr)){\
            __assertFailed(#expr, __func__, __FILE__, __LINE__);\
        }\
    }while(0)

static inline void __assertFailed(const char *expr, const char *func, const char *file, int line) {
    printf("Assertion failed: %s, at %s (%s:%d)", expr, func, file, line);
    disableCPU();
}

#endif

#endif
