
#ifndef JS_JS_H
#define JS_JS_H

#include "unicode/convert.h"
#include "data-struct/list.h"
#include "c/stdbool.h"

typedef struct struct_lex lex_t;

typedef double saki_number_t;
typedef utf16_string_t saki_string_t;
typedef bool saki_boolean_t;
typedef struct struct_saki_object *saki_object_t;

#define ASSIGN_FLAG 0x80
#define DOUBLE_FLAG 0x100
#define OTHER_FLAG 0x200
#define KEYWORD_FLAG 0x400

enum token_class_t {
    L_BRACE = '{',
    R_BRACE = '}',
    L_PAREN = '(',
    R_PAREN = ')',
    L_BRACKET = '[',
    R_BRACKET = ']',
    POINT = '.',
    SEMICOLON = ';',
    COMMA = ',',
    NOT = '~',
    QUESTION = '?',
    COLON = ':',

    LT = '<',
    GT = '>',

    MUL = '*',
    DIV = '/',
    MOD = '%',
    XOR = '^',
    ADD = '+',
    SUB = '-',
    AND = '&',
    OR = '|',
    ASSIGN = '=',
    L_NOT = '!',


    MUL_ASSIGN = MUL | ASSIGN_FLAG,
    DIV_ASSIGN = DIV | ASSIGN_FLAG,
    MOD_ASSIGN = MOD | ASSIGN_FLAG,
    XOR_ASSIGN = XOR | ASSIGN_FLAG,
    ADD_ASSIGN = ADD | ASSIGN_FLAG,
    SUB_ASSIGN = SUB | ASSIGN_FLAG,
    AND_ASSIGN = AND | ASSIGN_FLAG,
    OR_ASSIGN = OR | ASSIGN_FLAG,
    EQ = ASSIGN | ASSIGN_FLAG,
    INEQ = L_NOT | ASSIGN_FLAG,

    LTEQ = LT | ASSIGN_FLAG,
    GTEQ = GT | ASSIGN_FLAG,


    INC = ADD | DOUBLE_FLAG,
    DEC = SUB | DOUBLE_FLAG,
    L_AND = AND | DOUBLE_FLAG,
    L_OR = OR | DOUBLE_FLAG,

    FULL_EQ = OTHER_FLAG,
    FULL_INEQ,
    STR,
    NUM,
    ID,
    REGEXP,

    SHL,
    SHR,
    USHR,
    SHL_ASSIGN,
    SHR_ASSIGN,
    USHR_ASSIGN,

    BREAK = KEYWORD_FLAG,
    CASE,
    CATCH,
    CONTINUE,
    DEBUGGER,
    DEFAULT,
    DELETE,
    DO,
    ELSE,
    FINALLY,
    FOR,
    FUNCTION,
    IF,
    IN,
    INSTANCEOF,
    NEW,
    RETURN,
    SWITCH,
    THIS,
    THROW,
    TRY,
    TYPEOF,
    VAR,
    VOID,
    WHILE,
    WITH,

    END_OF_FILE,

    NULL_LIT,
    TRUE_LIT,
    FALSE_LIT,

    RESERVED_WORD,
    RESERVED_STRICT,
};

typedef struct struct_token {
    enum token_class_t type;
    union {
        utf16_string_t stringValue;
        saki_number_t numberValue;
    };
    ENABLE_LIST(list);
    bool lineBefore;
} token_t;

struct struct_lex {
    uint16_t (*next)(lex_t *lex);
    uint16_t (*lookahead)(lex_t *lex);
    token_t *(*state)(lex_t *lex);
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
        saki_number_t number;
    } data;
};

typedef struct struct_grammar grammar_t;

typedef struct {
    enum {
        SAKI_TYPE_NULL,
        SAKI_TYPE_BOOLEAN,
        SAKI_TYPE_STRING,
        SAKI_TYPE_NUMBER,
        SAKI_TYPE_UNDEFINED,
        SAKI_TYPE_OBJECT
    } type;
    union {
        saki_boolean_t booleanValue;
        saki_number_t numberValue;
        saki_string_t stringValue;
        saki_object_t objectValue;
    };
} saki_value_t;

#define SAKI_SET_NULL(target) do{target.type=SAKI_TYPE_NULL;}while(0);
#define SAKI_SET_BOOLEAN(target, value) do{target.type=SAKI_TYPE_BOOLEAN;target.booleanValue=value;}while(0);
#define SAKI_SET_STRING(target, value) do{target.type=SAKI_TYPE_STRING;target.stringValue=value;}while(0);
#define SAKI_SET_NUMBER(target, value) do{target.type=SAKI_TYPE_NUMBER;target.numberValue=value;}while(0);
#define SAKI_SET_UNDEFINED(target) do{target.type=SAKI_TYPE_UNDEFINED;}while(0);
#define SAKI_SET_OBJECT(target, value) do{target.type=SAKI_TYPE_NUMBER;target.objectValue=value;}while(0);

lex_t *lex_new(char *chr);
token_t *lex_next(lex_t *lex);
void examine_token(token_t *tok);
void lex_disposeToken(token_t *tk);

grammar_t *grammar_new(lex_t *lex);

#endif