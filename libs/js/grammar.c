#include "js/js.h"
#include "js/node.h"
#include "c/stdlib.h"
#include "unicode/hash.h"
#include "c/assert.h"

typedef struct struct_grammar grammar_t;

struct struct_grammar {
    lex_t *lex;
    list_t list;
    size_t listLen;
    bool noIn;
};

node_t *grammar_primaryExpr(grammar_t *gmr);
node_t *grammar_memberExpr(grammar_t *gmr);

node_t *grammar_assignExpr(grammar_t *gmr);
node_t *grammar_expr(grammar_t *gmr);


static node_t *grammar_exprStmt(grammar_t *gmr);
static node_t *grammar_debuggerStmt(grammar_t *gmr);

static node_t *grammar_arguments(grammar_t *gmr);
static node_t *grammar_argumentList(grammar_t *gmr);

static node_t *grammar_funcDecl(grammar_t *gmr);
static node_t *grammar_funcExpr(grammar_t *gmr);
static node_t *grammar_formalParamList(grammar_t *gmr);
static node_t *grammar_funcBody(grammar_t *gmr);

node_t *grammar_program(grammar_t *gmr);
static node_t *grammar_sourceElements(grammar_t *gmr);
static node_t *grammar_sourceElement(grammar_t *gmr);

static token_t *next(grammar_t *gmr) {
    if (gmr->listLen) {
        gmr->listLen--;
        list_t *removed = gmr->list.next;
        list_remove(removed);
        return GET_DATA(removed, token_t, list);
    } else {
        token_t *ret = lex_next(gmr->lex);
        return ret;
    }
}

static void pushback(grammar_t *gmr, token_t *token) {
    gmr->listLen++;
    list_addFirst(&gmr->list, GET_LIST(token, list));
}

static token_t *lookahead(grammar_t *gmr) {
    token_t *ret = next(gmr);
    pushback(gmr, ret);
    return ret;
}

static token_t *expect(grammar_t *gmr, uint16_t type) {
    token_t *ret = next(gmr);
    if (ret->type != type) {
        printf("SyntaxError: Encountered %d, but %d expected\n", ret->type, type);
        assert(0);
    }
    return ret;
}

static bool expectAndDispose(grammar_t *gmr, uint16_t type) {
    token_t *token = expect(gmr, type);
    if (token) {
        lex_disposeToken(token);
    }
    return !!token;
}

static void discard(grammar_t *gmr) {
    lex_disposeToken(next(gmr));
}

static node_t *createNode(enum node_class_t type, size_t size) {
    node_t *node = malloc(size);
    node->type = type;
    node->next = NULL;
    return node;
}

static void moveString(utf16_string_t *dest, utf16_string_t *src) {
    *dest = *src;
    src->str = NULL;
}

static bool expectSemicolon(grammar_t *gmr) {
    gmr->lex->regexp = true;
    token_t *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == SEMICOLON) {
        discard(gmr);
        return true;
    }
    if (next->type == R_BRACE || next->lineBefore) {
        return true;
    } else {
        assert(!"SyntaxError: Expected semicolon after statement.");
        return false;
    }
}

grammar_t *grammar_new(lex_t *lex) {
    grammar_t *gmr = malloc(sizeof(struct struct_grammar));
    gmr->lex = lex;
    list_empty(&gmr->list);
    gmr->listLen = 0;
    gmr->noIn = true;
    return gmr;
}

/**
 * PrimaryExpression := this                LOOKAHEAD(1)=THIS
 *                    | Identifier          LOOKAHEAD(1)=ID
 *                    | Literal             LOOKAHEAD(1)=NULL, TRUE, FALSE, NUM, STR, REGEXP
 *                    | ArrayLiteral        LOOKAHEAD(1)=L_BRACKET
 *                    | ObjectLiteral       LOOKAHEAD(1)=L_BRACE
 *                    | (Expression)        LOOKAHEAD(1)=L_PAREN
 */
node_t *grammar_primaryExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    switch (lookahead(gmr)->type) {
        case THIS:
            discard(gmr);
            return createNode(PRIMARY_EXPR_THIS_NODE, sizeof(node_t));
        case ID: {
            token_t *id = next(gmr);
            primary_expr_id_node_t *node = (primary_expr_id_node_t *)createNode(PRIMARY_EXPR_ID_NODE, sizeof(literal_node_t));
            moveString(&node->name, &id->stringValue);
            lex_disposeToken(id);
            return (node_t *)node;
        }
        case NULL_LIT: {
            discard(gmr);
            literal_node_t *node = (literal_node_t *)createNode(LITERAL_NODE, sizeof(literal_node_t));
            SAKI_SET_NULL(node->value);
            return (node_t *)node;
        }
        case TRUE_LIT: {
            discard(gmr);
            literal_node_t *node = (literal_node_t *)createNode(LITERAL_NODE, sizeof(literal_node_t));
            SAKI_SET_BOOLEAN(node->value, true);
            return (node_t *)node;
        }
        case FALSE_LIT: {
            discard(gmr);
            literal_node_t *node = (literal_node_t *)createNode(LITERAL_NODE, sizeof(literal_node_t));
            SAKI_SET_BOOLEAN(node->value, false);
            return (node_t *)node;
        }
        case NUM: {
            token_t *num = next(gmr);
            literal_node_t *node = (literal_node_t *)createNode(LITERAL_NODE, sizeof(literal_node_t));
            SAKI_SET_NUMBER(node->value, num->numberValue);
            lex_disposeToken(num);
            return (node_t *)node;
        }
        case STR: {
            token_t *str = next(gmr);
            literal_node_t *node = (literal_node_t *)createNode(LITERAL_NODE, sizeof(literal_node_t));
            SAKI_SET_STRING(node->value, str->stringValue);
            str->stringValue.str = NULL;
            lex_disposeToken(str);
            return (node_t *)node;
        }
        case REGEXP: {
            assert(0);
        }

        case L_PAREN: {
            discard(gmr);
            node_t *ret = grammar_expr(gmr);
            expectAndDispose(gmr, R_PAREN);
            return ret;
        }
        /* ArrayLiteral */
        /* ObjectLiteral */
        default:
            assert(0);
            return NULL;
    }
}

/**
 * MemberExpression :=(
 *      PrimaryExpression
            LOOKAHEAD(1)=THIS, ID, NULL, TRUE, FALSE, NUM, STR, REGEXP, L_BRACKET, L_BRACE, L_PAREN
 *      | FunctionExpression        LOOKAHEAD(1)=FUNCTION
 *      | new MemberExpression Arguments_opt    LOOKAHEAD(1)=NEW
 *  ) (
 *      [ Expression ]
 *      | . IdentifierName
 *  )*
 */
node_t *grammar_memberExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    node_t *cur;
    switch (lookahead(gmr)->type) {
        case FUNCTION: {
            assert(!"FunctionLiteral not supported yet");
            break;
        }
        case NEW: {
            discard(gmr);
            node_t *node = grammar_memberExpr(gmr);
            node_t *args;
            if (lookahead(gmr)->type == L_PAREN) {
                args = grammar_arguments(gmr);
            } else {
                args = NULL;
            }
            member_expr_new_node_t *newNode =
                (member_expr_new_node_t *)createNode(MEMBER_EXPR_NEW_NODE, sizeof(member_expr_new_node_t));
            newNode->expr = node;
            newNode->args = args;
            cur = (node_t *)newNode;
            break;
        }
        default:
            cur = grammar_primaryExpr(gmr);
            break;
    }
    gmr->lex->regexp = false;
    while (1) {
        switch (lookahead(gmr)->type) {
            case POINT: {
                discard(gmr);
                gmr->lex->parseId = false;
                token_t *id = expect(gmr, ID);
                gmr->lex->parseId = true;
                member_expr_dot_node_t *node =
                    (member_expr_dot_node_t *)createNode(MEMBER_EXPR_DOT_NODE, sizeof(member_expr_dot_node_t));
                node->expr = cur;
                moveString(&node->name, &id->stringValue);
                lex_disposeToken(id);
                cur = (node_t *)node;
                break;
            }
            case L_BRACKET: {
                discard(gmr);
                node_t *tk = grammar_expr(gmr);
                expectAndDispose(gmr, R_BRACKET);
                member_expr_bracket_node_t *node =
                    (member_expr_bracket_node_t *)createNode(MEMBER_EXPR_BRACKET_NODE, sizeof(member_expr_bracket_node_t));
                node->expr = cur;
                node->name = tk;
                cur = (node_t *)node;
                break;
            }
            default:
                return cur;
        }
    }
}

/*
 * Arguments := ( ArgumentList_opt )
 */
static node_t *grammar_arguments(grammar_t *gmr) {
    expectAndDispose(gmr, L_PAREN);
    if (lookahead(gmr)->type != R_PAREN) {
        node_t *ret = grammar_argumentList(gmr);
        expectAndDispose(gmr, R_PAREN);
        return ret;
    }
    discard(gmr);
    return NULL;
}

static node_t *grammar_argumentList(grammar_t *gmr) {
    node_t *first = grammar_assignExpr(gmr);
    node_t *cur = first;
    while (lookahead(gmr)->type == COMMA) {
        discard(gmr);
        node_t *next = grammar_assignExpr(gmr);
        cur->next = next;
        cur = next;
    }
    return first;
}

/*
 * LeftHandSideExpression := MemberExpression
 *      (
            Arguments
            | . IdentifierName
            | [ Expression ]
 *      )*
 */
node_t *grammar_leftHandSideExpr(grammar_t *gmr) {
    node_t *cur = grammar_memberExpr(gmr);
    while (1) {
        switch (lookahead(gmr)->type) {
            case POINT: {
                discard(gmr);
                gmr->lex->parseId = false;
                token_t *id = expect(gmr, ID);
                gmr->lex->parseId = true;
                member_expr_dot_node_t *node =
                    (member_expr_dot_node_t *)createNode(MEMBER_EXPR_DOT_NODE, sizeof(member_expr_dot_node_t));
                node->expr = cur;
                moveString(&node->name, &id->stringValue);
                lex_disposeToken(id);
                cur = (node_t *)node;
                break;
            }
            case L_BRACKET: {
                discard(gmr);
                node_t *tk = grammar_expr(gmr);
                expectAndDispose(gmr, R_BRACKET);
                member_expr_bracket_node_t *node =
                    (member_expr_bracket_node_t *)createNode(MEMBER_EXPR_BRACKET_NODE, sizeof(member_expr_bracket_node_t));
                node->expr = cur;
                node->name = tk;
                cur = (node_t *)node;
                break;
            }
            case L_PAREN: {
                node_t *args = grammar_arguments(gmr);
                call_expr_node_t *node =
                    (call_expr_node_t *)createNode(CALL_EXPR_NODE, sizeof(call_expr_node_t));
                node->expr = cur;
                node->args = args;
                cur = (node_t *)node;
                break;
            }
            default:
                return cur;
        }
    }
}

node_t *grammar_postfixExpr(grammar_t *gmr) {
    node_t *expr = grammar_leftHandSideExpr(gmr);

    token_t *next = lookahead(gmr);
    if (next->lineBefore) {
        return expr;
    }

    if (next->type == INC) {
        unary_node_t *node =
            (unary_node_t *)createNode(POSTFIX_EXPR_INC_NODE, sizeof(unary_node_t));
        node->first = expr;
        return (node_t *)node;
    } else if (next->type == DEC) {
        unary_node_t *node =
            (unary_node_t *)createNode(POSTFIX_EXPR_DEC_NODE, sizeof(unary_node_t));
        node->first = expr;
        return (node_t *)node;
    }

    return expr;
}

node_t *grammar_unaryExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    uint16_t nodeClass;
    token_t *lh = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (lh->type) {
        case DELETE:
            nodeClass = UNARY_EXPR_DELETE_NODE;
            goto produceExpr;
        case VOID:
            nodeClass = UNARY_EXPR_VOID_NODE;
            goto produceExpr;
        case TYPEOF:
            nodeClass = UNARY_EXPR_TYPEOF_NODE;
            goto produceExpr;
        case INC:
            nodeClass = UNARY_EXPR_INC_NODE;
            goto produceExpr;
        case DEC:
            nodeClass = UNARY_EXPR_DEC_NODE;
            goto produceExpr;
        case ADD:
            nodeClass = UNARY_EXPR_POS_NODE;
            goto produceExpr;
        case SUB:
            nodeClass = UNARY_EXPR_NEG_NODE;
            goto produceExpr;
        case NOT:
            nodeClass = UNARY_EXPR_NOT_NODE;
            goto produceExpr;
        case L_NOT:
            nodeClass = UNARY_EXPR_LNOT_NODE;
produceExpr:
            discard(gmr);
            unary_node_t *node =
                (unary_node_t *)createNode(nodeClass, sizeof(unary_node_t));
            node->first = grammar_unaryExpr(gmr);
            return (node_t *)node;
        default:
            return grammar_postfixExpr(gmr);
    }
}

#define BINARY_HEAD(_name, _previous) node_t *grammar_##_name(grammar_t *gmr) {\
        node_t *cur = grammar_##_previous(gmr);\
        while (true) {\
            uint16_t type;\
            switch (lookahead(gmr)->type) {

#define BINARY_CASE(_prefix, _value) case _value: {\
    type = _prefix##_##_value##_NODE;\
    break;\
}\
 
#define BINARY_FOOT(_previous) default:\
    return cur;\
    }\
    discard(gmr);\
    binary_node_t *node =\
                         (binary_node_t *)createNode(type, sizeof(binary_node_t));\
    node->first = cur;\
    node->second = grammar_##_previous(gmr);\
    cur = (node_t *)node;\
    }\
    }

#define BINARY_1(_name, _prefix, _previous, _first) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_prefix, _first)\
    BINARY_FOOT(_previous)

#define BINARY_2(_name, _prefix, _previous, _first, _second) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_prefix, _first)\
    BINARY_CASE(_prefix, _second)\
    BINARY_FOOT(_previous)

#define BINARY_3(_name, _prefix, _previous, _first, _second, _third) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_prefix, _first)\
    BINARY_CASE(_prefix, _second)\
    BINARY_CASE(_prefix, _third)\
    BINARY_FOOT(_previous)

BINARY_3(mulExpr, MUL_EXPR, unaryExpr, MUL, MOD, DIV);
BINARY_2(addExpr, ADD_EXPR, mulExpr, ADD, SUB);
BINARY_3(shiftExpr, SHIFT_EXPR, addExpr, SHL, SHR, USHR);

BINARY_HEAD(relExpr, shiftExpr)
BINARY_CASE(REL_EXPR, LT)
BINARY_CASE(REL_EXPR, GT)
BINARY_CASE(REL_EXPR, LTEQ)
BINARY_CASE(REL_EXPR, GTEQ)
BINARY_CASE(REL_EXPR, INSTANCEOF)
case IN: {
    if (gmr->noIn) {
        return cur;
    }
    type = REL_EXPR_IN_NODE;
    break;
}
BINARY_FOOT(shiftExpr);

BINARY_HEAD(eqExpr, relExpr)
BINARY_CASE(EQ_EXPR, EQ)
BINARY_CASE(EQ_EXPR, INEQ)
BINARY_CASE(EQ_EXPR, FULL_EQ)
BINARY_CASE(EQ_EXPR, FULL_INEQ)
BINARY_FOOT(relExpr)

BINARY_1(andExpr, BITWISE_EXPR, eqExpr, AND)
BINARY_1(xorExpr, BITWISE_EXPR, andExpr, XOR)
BINARY_1(orExpr, BITWISE_EXPR, xorExpr, OR)
BINARY_1(lAndExpr, LOGICAL_EXPR, orExpr, L_AND)
BINARY_1(lOrExpr, LOGICAL_EXPR, lAndExpr, L_OR)

static node_t *grammar_condExpr(grammar_t *gmr) {
    node_t *node = grammar_lOrExpr(gmr);
    if (lookahead(gmr)->type == QUESTION) {
        discard(gmr);
        node_t *t_exp = grammar_assignExpr(gmr);
        expectAndDispose(gmr, COLON);
        node_t *f_exp = grammar_assignExpr(gmr);
        cond_node_t *ret =
            (cond_node_t *)createNode(COND_EXPR_NODE, sizeof(cond_node_t));
        ret->expr = node;
        ret->true_expr = t_exp;
        ret->false_expr = f_exp;
        return (node_t *)ret;
    } else {
        return node;
    }
}

node_t *grammar_assignExpr(grammar_t *gmr) {
    node_t *node = grammar_condExpr(gmr);
    /* We do not care wether it is LeftHandSide. We can check it later */
    uint16_t type;
    switch (lookahead(gmr)->type) {
        case ASSIGN: type = ASSIGN_EXPR_ASSIGN_NODE; break;
        case MUL_ASSIGN: type = ASSIGN_EXPR_MUL_NODE; break;
        case DIV_ASSIGN: type = ASSIGN_EXPR_DIV_NODE; break;
        case MOD_ASSIGN: type = ASSIGN_EXPR_MOD_NODE; break;
        case ADD_ASSIGN: type = ASSIGN_EXPR_ADD_NODE; break;
        case SUB_ASSIGN: type = ASSIGN_EXPR_SUB_NODE; break;
        case SHL_ASSIGN: type = ASSIGN_EXPR_SHL_NODE; break;
        case SHR_ASSIGN: type = ASSIGN_EXPR_SHR_NODE; break;
        case USHR_ASSIGN: type = ASSIGN_EXPR_USHR_NODE; break;
        case AND_ASSIGN: type = ASSIGN_EXPR_AND_NODE; break;
        case XOR_ASSIGN: type = ASSIGN_EXPR_XOR_NODE; break;
        case OR_ASSIGN: type = ASSIGN_EXPR_OR_NODE; break;
        default: return node;
    }
    discard(gmr);
    binary_node_t *ass =
        (binary_node_t *)createNode(type, sizeof(binary_node_t));
    ass->first = node;
    ass->second = grammar_assignExpr(gmr);
    return (node_t *)ass;
}

BINARY_1(expr, EXPR, assignExpr, COMMA);

/*
 * Statement:= Block                    LOOKAHEAD(1)=L_BRACE
 *           | VariableStatement        LOOKAHEAD(1)=VAR
 *           | EmptyStatement           LOOKAHEAD(1)=SEMICOLON
 *           | ExpressionStatement      LOOKAHEAD(1)=Blah
 *           | IfStatement              LOOKAHEAD(1)=IF
 *           | IterationStatement       LOOKAHEAD(1)=
 *           | ContinueStatement        LOOKAHEAD(1)=CONTINUE
 *           | BreakStatement           LOOKAHEAD(1)=BREAK
 *           | ReturnStatement          LOOKAHEAD(1)=RETURN
 *           | WithStatement            LOOKAHEAD(1)=WITH
 *           | LabelledStatement        LOOKAHEAD(1)=ID         LOOKAHEAD(2)=
 *           | SwitchStatement          LOOKAHEAD(1)=SWITCH
 *           | ThrowStatement           LOOKAHEAD(1)=THROW
 *           | TryStatement             LOOKAHEAD(1)=TRY
 *           | DebuggerStatement        LOOKAHEAD(1)=DEBUGGER
*/
node_t *grammar_stmt(grammar_t *gmr) {
    gmr->lex->regexp = true;
    token_t *next = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (next->type) {
        case SEMICOLON:
            discard(gmr);
            return createNode(EMPTY_STMT_NODE, sizeof(node_t));
        case DEBUGGER:
            return grammar_debuggerStmt(gmr);
        default: {
            return grammar_exprStmt(gmr);
        }
    }
}

static node_t *grammar_exprStmt(grammar_t *gmr) {
    node_t *expr = grammar_expr(gmr);

    unary_node_t *node = (unary_node_t *)createNode(EXPR_STMT_NODE, sizeof(unary_node_t));
    node->first = expr;

    expectSemicolon(gmr);
    return node;
}

/*
 * DebuggerStatemenet := debugger ;         LOOKAHEAD(1)=DEBUGGER
 */
static node_t *grammar_debuggerStmt(grammar_t *gmr) {
    expectAndDispose(gmr, DEBUGGER);
    expectSemicolon(gmr);
    return createNode(DEBUGGER_NODE, sizeof(node_t));
}

static node_t *grammar_funcDecl(grammar_t *gmr) {
    function_node_t *func = (function_node_t *)grammar_funcExpr(gmr);
    assert(func->name.len);
    return (node_t *)func;
}

static node_t *grammar_funcExpr(grammar_t *gmr) {
    expectAndDispose(gmr, FUNCTION);
    function_node_t *func = (function_node_t *)createNode(FUNCTION_NODE, sizeof(function_node_t));

    if (lookahead(gmr)->type == ID) {
        token_t *id = next(gmr);
        moveString(&func->name, &id->stringValue);
        lex_disposeToken(id);
    } else {
        func->name.str = NULL;
        func->name.len = 0;
    }

    expectAndDispose(gmr, L_PAREN);

    if (lookahead(gmr)->type == ID) {
        func->args = grammar_formalParamList(gmr);
    } else {
        func->args = NULL;
    }

    expectAndDispose(gmr, R_PAREN);
    expectAndDispose(gmr, L_BRACE);
    func->body = grammar_funcBody(gmr);
    expectAndDispose(gmr, R_BRACE);

    return (node_t *)func;
}

/**
 * LOOKAHEAD(1) = Identifier
 *
 * FormalParameterList := Identifier {, Identifier}
 */
static node_t *grammar_formalParamList(grammar_t *gmr) {
    token_t *id = expect(gmr, ID);

    var_node_t *var = (var_node_t *)createNode(VAR_NODE, sizeof(var_node_t));
    moveString(&var->name, &id->stringValue);
    lex_disposeToken(id);
    var_node_t *last = var;

    while (lookahead(gmr)->type == COMMA) {
        token_t *nextId = expect(gmr, ID);
        var_node_t *cur = (var_node_t *)createNode(VAR_NODE, sizeof(var_node_t));
        moveString(&cur->name, &nextId->stringValue);
        lex_disposeToken(nextId);
        last->header.next = (node_t *)cur;
        last = cur;
    }

    return (node_t *)var;
}

static node_t *grammar_funcBody(grammar_t *gmr) {
    gmr->lex->regexp = true;
    token_t *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == END_OF_FILE || next->type == R_BRACE) {
        return NULL;
    }
    return grammar_sourceElements(gmr);
}

node_t *grammar_program(grammar_t *gmr) {
    return grammar_funcBody(gmr);
}

static node_t *grammar_sourceElements(grammar_t *gmr) {
    node_t *first = grammar_sourceElement(gmr);
    node_t *cur = first;
    while (true) {
        gmr->lex->regexp = true;
        token_t *next = lookahead(gmr);
        gmr->lex->regexp = false;
        if (next->type == END_OF_FILE || next->type == R_BRACE) {
            return first;
        }
        node_t *n = grammar_sourceElement(gmr);
        cur->next = n;
        cur = n;
    }
}

static node_t *grammar_sourceElement(grammar_t *gmr) {
    gmr->lex->regexp = true;
    token_t *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == FUNCTION) {
        return grammar_funcDecl(gmr);
    } else {
        return grammar_stmt(gmr);
    }
}
