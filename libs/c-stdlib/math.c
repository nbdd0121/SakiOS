#include "c/math.h"
#include "c/stdint.h"


static uint16_t getFPUControl(void) {
    uint16_t ret;
    __asm__ __volatile__ ("fstcw %0":"=m"(ret));
    return ret;
}

static void setFPUControl(uint16_t val) {
    __asm__ __volatile__ ("fldcw %0"::"m"(val));
}

int isnan(double x) {
    return x != x;
}

int isinf(double x) {
    union {
        double doubleValue;
        uint64_t intValue;
    } extracted = {
        .doubleValue = x
    };
    return extracted.intValue == UINT64_C(0x7FF0000000000000) ||
           extracted.intValue == UINT64_C(0xFFF0000000000000);
}

double fabs(double x) {
    return x > 0 ? x : -x;
}

double fmod(double x, double y) {
    __asm __volatile__ ("1: fprem; fnstsw %%ax; sahf; jp 1b;":"=t"(x):"0"(x), "u"(y):"ax", "cc");
    return x;
}

double log2(double x) {
    __asm __volatile__ ("fld1; fxch; fyl2x":"=t"(x):"0"(x));
    return x;
}

double log10(double x) {
    __asm __volatile__ ("fldlg2; fxch; fyl2x":"=t"(x):"0"(x));
    return x;
}

double log(double x) {
    __asm __volatile__ ("fldln2; fxch; fyl2x":"=t"(x):"0"(x));
    return x;
}

double exp2(double x) {
    __asm __volatile__ ("f2xm1;":"=t"(x):"0"(x));
    return x + 1;
}

double pow(double x, double y) {
    return exp2(log2(x) * y);
}

double floor(double x) {
    uint16_t ctrl = getFPUControl();
    setFPUControl((ctrl & ~0xC00) | 0x400);
    __asm __volatile__ ("frndint;":"=t"(x):"0"(x));
    setFPUControl(ctrl);
    return x;
}