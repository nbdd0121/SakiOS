#include "unicode/convert.h"
#include "js.h"

typedef struct struct_node_header node_t;

enum node_class_t {
    LITERAL_NODE,
    PRIMARY_EXPR_THIS_NODE,
    PRIMARY_EXPR_ID_NODE,
    MEMBER_EXPR_NEW_NODE,
    MEMBER_EXPR_DOT_NODE,
    MEMBER_EXPR_BRACKET_NODE,
    CALL_EXPR_NODE,
    POSTFIX_EXPR_INC_NODE,
    POSTFIX_EXPR_DEC_NODE,
    UNARY_EXPR_DELETE_NODE,
    UNARY_EXPR_VOID_NODE,
    UNARY_EXPR_TYPEOF_NODE,
    UNARY_EXPR_INC_NODE,
    UNARY_EXPR_DEC_NODE,
    UNARY_EXPR_POS_NODE,
    UNARY_EXPR_NEG_NODE,
    UNARY_EXPR_NOT_NODE,
    UNARY_EXPR_LNOT_NODE,
    MUL_EXPR_MUL_NODE,
    MUL_EXPR_MOD_NODE,
    MUL_EXPR_DIV_NODE,
    ADD_EXPR_ADD_NODE,
    ADD_EXPR_SUB_NODE,
    SHIFT_EXPR_SHL_NODE,
    SHIFT_EXPR_SHR_NODE,
    SHIFT_EXPR_USHR_NODE,
    REL_EXPR_LT_NODE,
    REL_EXPR_GT_NODE,
    REL_EXPR_LTEQ_NODE,
    REL_EXPR_GTEQ_NODE,
    REL_EXPR_INSTANCEOF_NODE,
    REL_EXPR_IN_NODE,
    EQ_EXPR_EQ_NODE,
    EQ_EXPR_INEQ_NODE,
    EQ_EXPR_FULL_EQ_NODE,
    EQ_EXPR_FULL_INEQ_NODE,
    BITWISE_EXPR_AND_NODE,
    BITWISE_EXPR_XOR_NODE,
    BITWISE_EXPR_OR_NODE,
    LOGICAL_EXPR_L_AND_NODE,
    LOGICAL_EXPR_L_OR_NODE,
    COND_EXPR_NODE,
    ASSIGN_EXPR_ASSIGN_NODE,
    ASSIGN_EXPR_MUL_NODE,
    ASSIGN_EXPR_DIV_NODE,
    ASSIGN_EXPR_MOD_NODE,
    ASSIGN_EXPR_ADD_NODE,
    ASSIGN_EXPR_SUB_NODE,
    ASSIGN_EXPR_SHL_NODE,
    ASSIGN_EXPR_SHR_NODE,
    ASSIGN_EXPR_USHR_NODE,
    ASSIGN_EXPR_AND_NODE,
    ASSIGN_EXPR_XOR_NODE,
    ASSIGN_EXPR_OR_NODE,
    EXPR_COMMA_NODE,

    VAR_STMT_NODE,

    EMPTY_STMT_NODE,
    EXPR_STMT_NODE,
    DEBUGGER_NODE,

    FUNCTION_NODE,

    VAR_NODE,
};

struct struct_node_header {
    enum node_class_t type;
    node_t *next;
};

typedef struct var {
    node_t header;
    utf16_string_t name;
} var_node_t, primary_expr_id_node_t;

typedef struct literal {
    node_t header;
    saki_value_t value;
} literal_node_t;

typedef struct member_expr_new {
    node_t header;
    node_t *expr;
    node_t *args;
} member_expr_new_node_t, call_expr_node_t;

typedef struct member_expr_dot {
    node_t header;
    node_t *expr;
    utf16_string_t name;
} member_expr_dot_node_t;

typedef struct member_expr_bracket {
    node_t header;
    node_t *expr;
    node_t *name;
} member_expr_bracket_node_t;

typedef struct {
    node_t header;
    node_t *first;
} unary_node_t;

typedef struct {
    node_t header;
    node_t *first;
    node_t *second;
} binary_node_t;

typedef struct {
    node_t header;
    node_t *expr;
    node_t *true_expr;
    node_t *false_expr;
} cond_node_t, if_node_t;

typedef struct {
    node_t header;
    utf16_string_t name;
    node_t *args;
    node_t *body;
} function_node_t;

