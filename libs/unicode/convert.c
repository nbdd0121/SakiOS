#include "c/stdlib.h"
#include "c/assert.h"
#include "unicode/convert.h"

#define BIT_TEST(num, bit) (!!((num)&(1<<bit)))

size_t unicode_countAsUtf16(utf8_string_t utf8) {
    size_t len = 0;
    for (int i = 0; i < utf8.len; i++, len++) {
        uint8_t c = utf8.str[i];
        if (BIT_TEST(c, 7)) {
            assert(BIT_TEST(c, 6));
            if (!BIT_TEST(c, 5)) {
                i++;
            } else if (!BIT_TEST(c, 4)) {
                i += 2;
            } else {
                assert(!BIT_TEST(c, 3));
                len++;
                i += 3;
            }
        }
    }
    return len;
}

size_t unicode_countAsUtf8(utf16_string_t utf16) {
    size_t len = 0;
    for (int i = 0; i < utf16.len; i++, len++) {
        uint16_t c = utf16.str[i];
        if (c < 0x7F) {
        } else if (c < 0x7FF) {
            len++;
        } else if (c >= 0xD800 && c < 0xDC00) {
            uint16_t n = utf16.str[++i];
            assert(n >= 0xDC00 && c < 0xE000);
            len += 3;
        } else {
            len += 2;
        }
    }
    return len;
}

utf16_string_t unicode_toUtf16(utf8_string_t utf8) {
    size_t expectedLen = unicode_countAsUtf16(utf8);
    uint16_t *result = malloc(expectedLen * sizeof(uint16_t));
    size_t len = 0;
    for (int i = 0; i < utf8.len; i++, len++) {
        uint8_t c = utf8.str[i];
        if (!BIT_TEST(c, 7)) {
            result[len] = c;
        } else {
            if (!BIT_TEST(c, 5)) {
                uint8_t n = utf8.str[++i];
                assert((n & 0xC0) == 0x80);
                result[len] = ((c & 31) << 6) | (n & 63);
            } else if (!BIT_TEST(c, 4)) {
                uint8_t n = utf8.str[++i];
                uint8_t nn = utf8.str[++i];
                assert((n & 0xC0) == 0x80 && (nn & 0xC0) == 0x80);
                result[len] = ((c & 15) << 12) | ((n & 63) << 6) | (nn & 63);
            } else {
                uint8_t n = utf8.str[++i];
                uint8_t nn = utf8.str[++i];
                uint8_t nnn = utf8.str[++i];
                assert((n & 0xC0) == 0x80 && (nn & 0xC0) == 0x80 && (nnn & 0xC0) == 0x80);
                uint32_t codePoint = (((c & 7) << 18) | ((n & 63) << 12) | ((nn & 63) << 6) | (nnn & 63)) - 0x10000;
                result[len] = 0xD800 | (codePoint >> 10);
                result[++len] = 0xDC00 | (codePoint & 0x3FF);
            }
        }
    }
    assert(len == expectedLen);
    return (utf16_string_t) {
        .str = result,
         .len = len
    };
}

utf8_string_t unicode_toUtf8(utf16_string_t utf16) {
    size_t expectedLen = unicode_countAsUtf8(utf16);
    uint8_t *result = malloc(expectedLen);
    size_t len = 0;
    for (int i = 0; i < utf16.len; i++, len++) {
        uint16_t c = utf16.str[i];
        if (c < 0x7F) {
            result[len] = c;
        } else if (c < 0x7FF) {
            result[len] = (c >> 6) | 0xC0;
            result[++len] = (c & 0x3F) | 0x80;
        } else if (c >= 0xD800 && c < 0xDC00) {
            uint16_t n = utf16.str[++i];
            uint32_t codePoint = (((c - 0xD800) << 10) | (n - 0xDC00)) + 0x10000;
            result[len] = (codePoint >> 18) | 0xF0;
            result[++len] = ((codePoint >> 12) & 0x3F) | 0x80;
            result[++len] = ((codePoint >> 6) & 0x3F) | 0x80;
            result[++len] = (codePoint & 0x3F) | 0x80;
        } else {
            result[len] = (c >> 12) | 0xE0;
            result[++len] = ((c >> 6) & 0x3F) | 0x80;
            result[++len] = (c & 0x3F) | 0x80;
        }
    }
    assert(len == expectedLen);
    return (utf8_string_t) {
        .str = result,
         .len = len
    };
}

void unicode_putUtf8(utf8_string_t utf8) {
    printf("%.*s", utf8.len, utf8.str);
}

void unicode_putUtf16(utf16_string_t utf16) {
    unicode_putUtf8(unicode_toUtf8(utf16));
}
