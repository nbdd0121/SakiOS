
#include "unicode/convert.h"

typedef struct struct_lex lex_t;

typedef double sakijs_number_t;

#define ASSIGN_FLAG 0x80
#define DOUBLE_FLAG 0x100
#define OTHER_FLAG 0x200

typedef struct struct_token {
    enum {
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
        LINE,
        SHL,
        SHR,
        USHR,
        SHL_ASSIGN,
        SHR_ASSIGN,
        USHR_ASSIGN,
    } type;
    union {
        utf16_string_t stringValue;
        sakijs_number_t numberValue;
    };
} token_t;

lex_t *lex_new(char *chr);
token_t *lex_next(lex_t *lex);