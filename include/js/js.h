
#ifndef JS_JS_H
#define JS_JS_H

#include "unicode/convert.h"
#include "c/stdbool.h"
#include "js/type.h"

typedef struct struct_lex lex_t;

struct struct_lex {
    uint16_t (*next)(lex_t *lex);
    uint16_t (*lookahead)(lex_t *lex);
    js_token_t *(*state)(lex_t *lex);
    utf16_string_t content;
    size_t ptr;
    bool regexp;
    bool strictMode;
    bool lineBefore;
    bool parseId;
    union {
        struct {
            uint16_t *buffer;
            size_t size;
            size_t length;
        };
        double number;
    } data;
};

typedef struct struct_grammar grammar_t;

lex_t *lex_new(char *chr);
js_token_t *lex_next(lex_t *lex);

grammar_t *grammar_new(lex_t *lex);

#endif
