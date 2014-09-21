#ifndef JS_TYPE_H
#define JS_TYPE_H

#include "c/stdbool.h"
#include "data-struct/hashmap.h"
#include "unicode/convert.h"

enum js_data_type_t {
    JS_NULL,
    JS_UNDEFINED,
    JS_BOOLEAN,
    JS_STRING,
    JS_NUMBER,
    JS_OBJECT,

    JS_INTERNAL_REF,
    JS_INTERNAL_COMPLETION,
    JS_INTERNAL_PROPERTY,

    JS_INTERNAL_TOKEN,
    JS_INTERNAL_EMPTY_NODE,
    JS_INTERNAL_UNARY_NODE,
    JS_INTERNAL_BINARY_NODE,
    JS_INTERNAL_TERNARY_NODE
};

#define ASSIGN_FLAG 0x80
#define DOUBLE_FLAG 0x100
#define OTHER_FLAG 0x200
#define KEYWORD_FLAG 0x400

enum js_token_type_t {
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

enum js_empty_node_type_t {
    THIS_NODE,
};

enum js_unary_node_type_t {
    POST_INC_NODE,
    POST_DEC_NODE,
    DELETE_NODE,
    VOID_NODE,
    TYPEOF_NODE,
    PRE_INC_NODE,
    PRE_DEC_NODE,
    POS_NODE,
    NEG_NODE,
    NOT_NODE,
    LNOT_NODE,

    EXPR_STMT
};

enum js_binary_node_type_t {
    MEMBER_NODE,

    MUL_NODE,
    MOD_NODE,
    DIV_NODE,
    ADD_NODE,
    SUB_NODE,
    SHL_NODE,
    SHR_NODE,
    USHR_NODE,
    LT_NODE,
    GT_NODE,
    LTEQ_NODE,
    GTEQ_NODE,
    INSTANCEOF_NODE,
    IN_NODE,
    EQ_NODE,
    INEQ_NODE,
    FULL_EQ_NODE,
    FULL_INEQ_NODE,
    AND_NODE,
    XOR_NODE,
    OR_NODE,
    L_AND_NODE,
    L_OR_NODE,

    ASSIGN_NODE,
    MUL_ASSIGN_NODE,
    DIV_ASSIGN_NODE,
    MOD_ASSIGN_NODE,
    ADD_ASSIGN_NODE,
    SUB_ASSIGN_NODE,
    SHL_ASSIGN_NODE,
    SHR_ASSIGN_NODE,
    USHR_ASSIGN_NODE,
    AND_ASSIGN_NODE,
    XOR_ASSIGN_NODE,
    OR_ASSIGN_NODE,

    COMMA_NODE,
};

enum js_ternary_node_type_t {
    COND_NODE
};

typedef struct js_data_t {
    enum js_data_type_t type;
    uint8_t flag;
} js_data_t;

typedef js_data_t js_null_t, js_undefined_t, js_boolean_t;

typedef struct js_string_t {
    js_data_t header;
    utf16_string_t value;
} js_string_t;

typedef struct js_number_t {
    js_data_t header;
    double value;
} js_number_t;

typedef struct js_object_t js_object_t;

typedef struct js_property_t {
    js_data_t header;
    js_data_t *value;
    js_data_t *get;
    js_data_t *set;
    js_data_t *writable;
    js_data_t *enumerable;
    js_data_t *configurable;
} js_property_t;

struct js_object_t {
    js_data_t header;
    hashmap_t *properties;
    js_object_t *prototype;
    js_string_t *clazz;
    bool extensible;
    js_data_t *(*get)(js_object_t *, js_string_t *);
    js_property_t *(*getOwnProperty)(js_object_t *, js_string_t *);
    js_property_t *(*getProperty)(js_object_t *, js_string_t *);
    void (*put)(js_object_t *, js_string_t *, js_data_t *, bool);
    bool (*canPut)(js_object_t *, js_string_t *);
    bool (*hasProperty)(js_object_t *, js_string_t *);
    bool (*_delete)(js_object_t *, js_string_t *, bool);
    js_data_t *(*defaultValue)(js_object_t *, enum js_data_type_t);
    bool (*defineOwnProperty)(js_object_t *, js_string_t *, js_property_t *, bool);
};

typedef struct js_context_t {
    js_data_t header;
    js_object_t *thisBinding;
} js_context_t;

typedef struct js_completion_t {
    js_data_t header;
    js_data_t *value;
    js_string_t *target;
    enum js_completion_type_t {
        COMPLETION_NORMAL,
        COMPLETION_BREAK,
        COMPLETION_CONTINUE,
        COMPLETION_RETURN,
        COMPLETION_THROW
    } type;
} js_completion_t;

typedef struct js_reference_t {
    js_data_t header;
    js_data_t *base;
    js_string_t *ref;
    bool strict;
} js_reference_t;

typedef struct js_token_t {
    js_data_t header;
    enum js_token_type_t type;
    js_data_t *value;
    struct js_token_t *next;
    bool lineBefore;
} js_token_t;

typedef struct js_empty_node_t {
    js_data_t header;
    enum js_empty_node_type_t type;
} js_empty_node_t;

typedef struct js_unary_node_t {
    js_data_t header;
    enum js_unary_node_type_t type;
    js_data_t *_1;
} js_unary_node_t;

typedef struct js_binary_node_t {
    js_data_t header;
    enum js_binary_node_type_t type;
    js_data_t *_1;
    js_data_t *_2;
} js_binary_node_t;

typedef struct js_ternary_node_t {
    js_data_t header;
    enum js_ternary_node_type_t type;
    js_data_t *_1;
    js_data_t *_2;
    js_data_t *_3;
} js_ternary_node_t;

extern js_data_t *js_constNull;
extern js_data_t *js_constUndefined;
extern js_data_t *js_constTrue;
extern js_data_t *js_constFalse;
extern js_data_t *js_constNaN;
extern js_data_t *js_constZero;
extern js_data_t *js_constOne;
extern js_string_t *js_constNullStr;
extern js_string_t *js_constUndefStr;
extern js_string_t *js_constTrueStr;
extern js_string_t *js_constFalseStr;
extern js_string_t *js_constNaNStr;
extern js_string_t *js_constInfStr;
extern js_string_t *js_constNegInfStr;
extern js_string_t *js_constZeroStr;

js_data_t *js_getValue(js_data_t *value);
void js_putValue(js_data_t *arg0, js_data_t *W);

/* ECMA-262 Ch 8 */
/* 8.10.1 */ bool js_isAccessorDescriptor(js_property_t *desc);
/* 8.10.2 */ bool js_isDataDescriptor(js_property_t *desc);
/* 8.10.3 */ bool js_isGenericDescriptor(js_property_t *desc);


/* ECMA-262 Ch 9 */
js_data_t *js_toPrimitive(js_data_t *value);
js_data_t *js_toBoolean(js_data_t *value);
js_data_t *js_toNumber(js_data_t *value);
//TODO toInteger
int32_t js_toInt32(js_data_t *value);
uint32_t js_toUint32(js_data_t *value);
int16_t js_toInt16(js_data_t *value);
js_string_t *js_toString(js_data_t *value);
void js_checkObjectCoercible(js_data_t *value);

js_data_t *js_new_number(double value);
js_string_t *js_new_string(utf16_string_t str);

js_data_t *js_alloc(enum js_data_type_t);
js_object_t *js_allocObject(void);
js_reference_t *js_allocReference(js_data_t *base, js_string_t *refName, bool strict);
js_completion_t *js_allocCompletion(enum js_completion_type_t type);
js_property_t *js_allocPropertyDesc(void);
js_token_t *js_allocToken(enum js_token_type_t type);
js_empty_node_t *js_allocEmptyNode(enum js_empty_node_type_t type);
js_unary_node_t *js_allocUnaryNode(enum js_unary_node_type_t type);
js_binary_node_t *js_allocBinaryNode(enum js_binary_node_type_t type);
js_ternary_node_t *js_allocTernaryNode(enum js_ternary_node_type_t type);

js_data_t *js_evalNode(js_context_t *context, js_data_t *node);

#define js_isTrue(bool) ((bool)==js_constTrue)

#endif

