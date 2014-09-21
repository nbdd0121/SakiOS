/**
 * Provide unicode category groups
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef UNICODE_TYPE_H
#define UNICODE_TYPE_H

#include "c/stdint.h"

enum {
    /* Cn */
    UNASSIGNED = 0,
    /* Lu */
    UPPERCASE_LETTER = 1,
    /* Ll */
    LOWERCASE_LETTER = 2,
    /* Lt */
    TITLECASE_LETTER = 3,
    /* Lm */
    MODIFIER_LETTER = 4,
    /* Lo */
    OTHER_LETTER = 5,
    /* Mn */
    NON_SPACING_MARK = 6,
    /* Me */
    ENCLOSING_MARK = 7,
    /* Mc */
    COMBINING_SPACING_MARK = 8,
    /* Nd */
    DECIMAL_DIGIT_NUMBER        = 9,
    /* Nl */
    LETTER_NUMBER = 10,
    /* No */
    OTHER_NUMBER = 11,
    /* Zs */
    SPACE_SEPARATOR = 12,
    /* Zl */
    LINE_SEPARATOR = 13,
    /* Zp */
    PARAGRAPH_SEPARATOR = 14,
    /* Cc */
    CONTROL = 15,
    /* Cf */
    FORMAT = 16,
    /* Co */
    PRIVATE_USE = 18,
    /* Cs */
    SURROGATE = 19,
    /* Pd */
    DASH_PUNCTUATION = 20,
    /* Ps */
    START_PUNCTUATION = 21,
    /* Pe */
    END_PUNCTUATION = 22,
    /* Pc */
    CONNECTOR_PUNCTUATION = 23,
    /* Po */
    OTHER_PUNCTUATION = 24,
    /* Sm */
    MATH_SYMBOL = 25,
    /* Sc */
    CURRENCY_SYMBOL = 26,
    /* Sk */
    MODIFIER_SYMBOL = 27,
    /* So */
    OTHER_SYMBOL = 28,
    /* Pi */
    INITIAL_QUOTE_PUNCTUATION = 29,
    /* Pf */
    FINAL_QUOTE_PUNCTUATION = 30,
};

uint8_t unicode_getType(uint32_t ch);

#endif