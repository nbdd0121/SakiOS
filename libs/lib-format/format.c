/**
 * Provide formatter support for c stdio library
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/stdio.h"
#include "c/stdbool.h"
#include "c/string.h"
#include "c/stdarg.h"

#define LEFT 1
#define PLUS 2
#define SPACE 4
#define SPECIAL 8
#define ZEROPAD 16
#define SIGN 32
#define SMALL 64

/* Important: these function maybe moved and changed, the declaration are not safe */
static char *itoa(int32_t num, char *str, int32_t radix, int32_t width, int32_t precision, int32_t type);
static int32_t atoi(char *p);

static char *itoa(int32_t num, char *str, int32_t radix, int32_t width, int32_t precision, int32_t type) {
    const char *index = (type & SMALL) ? "0123456789abcdefghijklmnopqrstuvwxyz" : "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t unum;
    int32_t i = 0, j, k;
    if ((type & (SIGN | PLUS)) && num < 0) {
        unum = (size_t) - num;
        str[i++] = '-';
    } else if (type & PLUS) {
        unum = (size_t)num;
        str[i++] = '+';
    } else
        unum = (size_t)num;
    if (type & SPECIAL) {
        if (radix == 8)str[i++] = '0';
        else if (radix == 16) {
            str[i++] = '0';
            str[i++] = (type & SMALL) ? 'x' : 'X';
        }
    }
    do {
        str[i++] = index[unum % (unsigned)radix];
        unum /= radix;
    } while (unum);
    if (str[0] == '-' || str[0] == '+')
        k = 1;
    else k = 0;
    k += (type & SPECIAL) ? (radix == 8 ? 1 : (radix == 16 ? 2 : 0)) : 0;
    char temp;
    if (!(type & LEFT))
        while (i < width) {
            str[i++] = (type & ZEROPAD) ? '0' : ' ';
        }
    for (j = k; j <= (i + k - 1) / 2; j++) {
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - j - 1 + k] = temp;
    }
    while (i < width) {
        str[i++] = ' ';
    }
    return str + i;
}

__attribute__((unused))
static int32_t atoi(char *p) {
    bool neg = false;
    int32_t res = 0;
    if (p[0] == '+' || p[0] == '-')
        neg = (*p++ != '+');
    while (*p >= '0' && *p <= '9')
        res = res * 10 + (*p++ -'0');
    return neg ? -res : res;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    char *p;
    char tmp[256];

    for (p = buf; *fmt != 0; fmt++) {
        if (*fmt != '%') {
            *p++ = *fmt;
            continue;
        }

        int32_t flags = 0;
        while (1) {
            fmt++;
            switch (*fmt) {
                case '-': flags |= LEFT; continue;
                case '+': flags |= PLUS; continue;
                case ' ': flags |= SPACE; continue;
                case '#': flags |= SPECIAL; continue;
                case '0': flags |= ZEROPAD; continue;
            }
            break;
        }

        int32_t field_width = -1;
        if (*fmt >= '0' && *fmt <= '9') {
            field_width = *fmt - '0';
            fmt++;
            while (*fmt >= '0' && *fmt <= '9') {
                field_width = field_width * 10 + *fmt - '0';
                fmt++;
            }
        } else if (*fmt == '*') {
            field_width = va_arg(args, int32_t);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
            fmt++;
        }

        int32_t precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (*fmt >= '0' && *fmt <= '9') {
                precision = *fmt - '0';
                fmt++;
            } else if (*fmt == '*') {
                precision = va_arg(args, int32_t);
                fmt++;
            }
            if (precision < 0)
                precision = 0;
        }

        __attribute__((unused))
        int32_t qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            ++fmt;
        }

        switch (*fmt) {
            case 'x': flags |= SMALL;
            case 'X':
                p = itoa(va_arg(args, uint32_t), p, 16, field_width, precision, flags);
                break;
            case 'd': case 'i': flags |= SIGN;
            case 'u':
                p = itoa(va_arg(args, int32_t), p, 10, field_width, precision, flags);
                break;
            case 'o':
                p = itoa(va_arg(args, size_t), p, 8, field_width, precision, flags);
                break;
            case 'n':;
                int *ip = va_arg(args, int *);
                *ip = p - tmp;
                break;
            case 'p':
                p = itoa(va_arg(args, size_t), p, 16, field_width, precision, flags);
                break;
            case 'c':
                if (!(flags & LEFT))
                    while (--field_width > 0)
                        *p++ = ' ';
                *p++ = va_arg(args, int);
                while (--field_width > 0)
                    *p++ = ' ';
                break;
            case 's':;
                char *str = va_arg(args, char *);
                /*size_t len=strlen(str);
                if(precision>=0&&(int)len>precision)
                    len=precision;*/
                size_t len = precision >= 0 ? strnlen(str, precision) : strlen(str);
                if (!(flags & LEFT))
                    while ((int)len < field_width--)
                        *p++ = ' ';
                memcpy(p, str, len);
                p += len;
                while ((int)len < field_width--)
                    *p++ = ' ';
                break;
            case '%':
                *p++ = '%';
                break;
            default:
                *p++ = '%';
                fmt--;
                break;
        }
    }
    *p = 0;
    return p - buf;
}

int printf(const char *fmt, ...) {
    size_t i;
    char buf[256];

    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    puts(buf);

    return i;
}

int sprintf(char *dest, const char *fmt, ...) {
    size_t i;

    va_list args;
    va_start(args, fmt);
    i = vsprintf(dest, fmt, args);
    va_end(args);
    dest[i] = 0;

    return i;
}


