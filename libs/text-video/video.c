#include "c/stdint.h"
#include "c/stdio.h"
#include "c/string.h"

#define inline inline __attribute__((always_inline))
#define VIDEO_ADDRESS 0xB8000
#define CHAR_PER_LINE 80
#define LINE_PER_SCREEN 25

static size_t x;
static size_t y;

static inline size_t calcOffset(size_t x, size_t y) {
    return y * 160 + x * 2;
}

static void lineWrap() {
    memmove(
        (void *)VIDEO_ADDRESS,
        (void *)(VIDEO_ADDRESS + CHAR_PER_LINE * 2),
        CHAR_PER_LINE * (LINE_PER_SCREEN - 1) * 2
    );
    memset(
        (void *)(VIDEO_ADDRESS + CHAR_PER_LINE * (LINE_PER_SCREEN - 1) * 2),
        0,
        CHAR_PER_LINE * 2
    );
}

int putchar(int character) {
    switch (character) {
        case '\r': x = 0; break;
        case '\n': {
            x = 0;
            y++;
            if (y == LINE_PER_SCREEN) {
                lineWrap();
                y--;
            }
            break;
        }
        case '\t': {
            x = (x + 4) / 4 * 4;
            if (x == CHAR_PER_LINE) {
                x = 0;
                y++;
            }
            if (y == LINE_PER_SCREEN) {
                lineWrap();
                y--;
            }
            break;
        }
        case '\f': {
            memset((void *)VIDEO_ADDRESS, 0, CHAR_PER_LINE * LINE_PER_SCREEN * 2);
            x = y = 0;
            break;
        }
        default: {
            *(uint16_t *)(VIDEO_ADDRESS + calcOffset(x, y)) = character | (0xF << 8);
            x++;
            if (x == CHAR_PER_LINE) {
                x = 0;
                y++;
            }
            if (y == LINE_PER_SCREEN) {
                lineWrap();
                y--;
            }
            break;
        }
    }
    return character;
}
