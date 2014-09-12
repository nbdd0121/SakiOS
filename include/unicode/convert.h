/**
 * Provide unicode convertion features and data types
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef UNICODE_CONVERT_H
#define UNICODE_CONVERT_H

#include "c/stdint.h"
#include "c/string.h"

typedef struct {
    uint8_t *str;
    size_t len;
} utf8_string_t;

typedef struct {
    uint16_t *str;
    size_t len;
} utf16_string_t;

#define UTF8_STRING(str_lit) ((utf8_string_t){.str=(uint8_t*)(str_lit), .len=strlen(str_lit)})

size_t unicode_countAsUtf16(utf8_string_t utf8);
size_t unicode_countAsUtf8(utf16_string_t utf16);
utf16_string_t unicode_toUtf16(utf8_string_t utf8);
utf8_string_t unicode_toUtf8(utf16_string_t utf16);
void unicode_putUtf8(utf8_string_t utf8);
void unicode_putUtf16(utf16_string_t utf16);

#endif