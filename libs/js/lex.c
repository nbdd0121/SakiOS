#include "unicode/type.h"
#include "unicode/convert.h"

#include "c/stdlib.h"
#include "c/stdio.h"
#include "c/assert.h"

typedef struct struct_lex lex_t;

struct struct_lex {
    uint16_t (*next)(lex_t *lex);
    uint16_t (*lookahead)(lex_t *lex);
    void (*state)(lex_t *lex);
    utf16_string_t content;
    size_t ptr;
    struct {
        uint16_t *buffer;
        size_t size;
        size_t length;
    } data;
};

enum {
    TAB = 0x9,
    VT = 0xB,
    FF = 0xC,
    SP = 0x20,
    NBSP = 0xA0,
    BOM = 0xFEFF,

    LF = 0xA,
    CR = 0xD,
    LS = 0x2028,
    PS = 0x2029,

    ZWNJ = 0x200C,
    ZWJ = 0x200D
};

static uint16_t lookahead(lex_t *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr];
}

static uint16_t next(lex_t *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr++];
}

static void createBuffer(lex_t *lex) {
    lex->data.length = 0;
    lex->data.size = 10;
    lex->data.buffer = malloc(10 * sizeof(uint16_t));
}

static void appendToBuffer(lex_t *lex, uint16_t ch) {
    if (lex->data.length == lex->data.size) {
        lex->data.size *= (lex->data.size * 3) / 2 + 1;
        lex->data.buffer = realloc(lex->data.buffer, lex->data.size * sizeof(uint16_t));
    }
    lex->data.buffer[lex->data.length++] = ch;
}

static utf16_string_t cleanBuffer(lex_t *lex) {
    utf16_string_t ret = {
        .str = lex->data.buffer,
        .len = lex->data.length
    };
    lex->data.size = 0;
    lex->data.length = 0;
    lex->data.buffer = 0;
    return ret;
}

static void stateDefault(lex_t *lex);
static void stateSingleLineComment(lex_t *lex);
static void stateMultiLineComment(lex_t *lex);
static void stateMultiLineCommentL(lex_t *lex);
static void stateIdentiferPart(lex_t *lex);

static void stateDefault(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case TAB:
        case VT:
        case FF:
        case SP:
        case NBSP:
        case BOM: {
            printf("[WS]");
            return;
        }
        case CR: {
            if (lex->lookahead(lex) == LF) {
                lex->next(lex);
            }
        }
        case LF:
        case LS:
        case PS: {
            printf("[LT]");
            return;
        }
        case '/': {
            uint16_t next = lex->lookahead(lex);
            if (next == '/') {
                lex->state = stateSingleLineComment;
            } else if (next == '*') {
                lex->state = stateMultiLineComment;
            } else {
                assert(0);
            }
            return;
        }
        case '$':
        case '_': {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return;
        }
        case '\\':
            // Unicode Escape Sequence
            assert(0);
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
        case '.':
        case ';':
        case ',':
        case '~':
        case '?':
        case ':': {
            printf("[PUNC %c]", next);
            return;
        }
        case '<': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                printf("[PUNC <=]");
                return;
            } else if (nch == '<') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    printf("[PUNC <<=]");
                    return;
                } else {
                    printf("[PUNC <<]");
                    return;
                }
            } else {
                printf("[PUNC <]");
                return;
            }
        }
        case '>': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                printf("[PUNC >=]");
                return;
            } else if (nch == '>') {
                lex->next(lex);
                uint16_t n2ch = lex->lookahead(lex);
                if (n2ch == '=') {
                    lex->next(lex);
                    printf("[PUNC >>=]");
                    return;
                } else if (n2ch == '>') {
                    lex->next(lex);
                    if (lex->lookahead(lex) == '=') {
                        lex->next(lex);
                        printf("[PUNC >>>=]");
                        return;
                    } else {
                        printf("[PUNC >>>]");
                        return;
                    }
                } else {
                    printf("[PUNC >>]");
                    return;
                }
            } else {
                printf("[PUNC >]");
                return;
            }
        }
        case '=':
        case '!': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    printf("[PUNC %c==]", next);
                    return;
                } else {
                    printf("[PUNC %c=]", next);
                    return;
                }
            } else {
                printf("[PUNC %c]", next);
                return;
            }
        }
        case '+':
        case '-':
        case '&':
        case '|': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                printf("[PUNC %c=]", next);
                return;
            } else if (nch == next) {
                lex->next(lex);
                printf("[PUNC %c%c]", next, next);
                return;
            } else {
                printf("[PUNC %c]", next);
                return;
            }
        }
        case '*':
        case '%':
        case '^': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                printf("[PUNC %c=]", next);
                return;
            } else {
                printf("[PUNC %c]", next);
                return;
            }
        }
    }

    switch (unicode_getType(next)) {
        case SPACE_SEPARATOR: {
            printf("[WS]");
            return;
        }
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER: {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return;
        }
    }
    assert(0);
}

static void stateSingleLineComment(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->state = stateDefault;
            printf("[WS]");
            return;
        }
    }
    lex->next(lex);
}

static void stateMultiLineComment(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '*': {
            if (lex->lookahead(lex) == '/') {
                lex->next(lex);
                lex->state = stateDefault;
                printf("[WS]");
            }
            return;
        }
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->state = stateMultiLineCommentL;
        }
    }
}

static void stateMultiLineCommentL(lex_t *lex) {
    uint16_t next = lex->next(lex);
    if (next == '*') {
        if (lex->lookahead(lex) == '/') {
            lex->next(lex);
            lex->state = stateDefault;
            printf("[LT]");
        }
        return;
    }
}

static void stateIdentiferPart(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case '$':
        case '_':
        case ZWNJ:
        case ZWJ: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return;
        }
        case '\\': {
            //Unicode Escape Sequence
            assert(0);
        }
    }
    switch (unicode_getType(next)) {
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER:
        case CONNECTOR_PUNCTUATION:
        case DECIMAL_DIGIT_NUMBER:
        case NON_SPACING_MARK:
        case COMBINING_SPACING_MARK: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return;
        }
    }
    lex->state = stateDefault;
    utf16_string_t identifierName = cleanBuffer(lex);
    utf8_string_t string = unicode_toUtf8(identifierName);
    printf("[Identifier %.*s]", string.len, string.str);
}

lex_t *lex_new(char *chr) {
    lex_t *l = malloc(sizeof(struct struct_lex));
    l->next = next;
    l->lookahead = lookahead;
    l->state = stateDefault;
    l->content = unicode_toUtf16(UTF8_STRING(chr));
    l->ptr = 0;
    return l;
}

void lex_next(lex_t *lex) {
    while (1)
        lex->state(lex);
}
