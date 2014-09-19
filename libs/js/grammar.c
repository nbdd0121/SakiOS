#include "js/js.h"
#include "c/stdlib.h"
#include "unicode/hash.h"
#include "c/assert.h"

typedef struct struct_grammar grammar_t;

struct struct_grammar {
    lex_t *lex;
    js_token_t *next;
    size_t listLen;
    bool noIn;
};

#include "js/type.h"

static js_data_t *grammar_primaryExpr(grammar_t *gmr);
static js_data_t *grammar_memberExpr(grammar_t *gmr);
static js_data_t *grammar_arguments(grammar_t *gmr);
static js_data_t *grammar_argumentList(grammar_t *gmr);
static js_data_t *grammar_leftHandSideExpr(grammar_t *gmr);
static js_data_t *grammar_postfixExpr(grammar_t *gmr);
static js_data_t *grammar_assignExpr(grammar_t *gmr);
js_data_t *grammar_expr(grammar_t *gmr);


static js_data_t *grammar_exprStmt(grammar_t *gmr);
static js_data_t *grammar_debuggerStmt(grammar_t *gmr);

static js_data_t *grammar_funcDecl(grammar_t *gmr);
static js_data_t *grammar_funcExpr(grammar_t *gmr);
static js_data_t *grammar_formalParamList(grammar_t *gmr);
static js_data_t *grammar_funcBody(grammar_t *gmr);

js_data_t *grammar_program(grammar_t *gmr);
static js_data_t *grammar_sourceElements(grammar_t *gmr);
static js_data_t *grammar_sourceElement(grammar_t *gmr);

static js_token_t *next(grammar_t *gmr) {
    if (gmr->listLen) {
        gmr->listLen--;
        js_token_t *removed = gmr->next;
        gmr->next = removed->next;
        return removed;
    } else {
        js_token_t *ret = lex_next(gmr->lex);
        return ret;
    }
}

static void pushback(grammar_t *gmr, js_token_t *token) {
    gmr->listLen++;
    token->next = gmr->next;
    gmr->next = token;
}

static js_token_t *lookahead(grammar_t *gmr) {
    js_token_t *ret = next(gmr);
    pushback(gmr, ret);
    return ret;
}

static js_token_t *expect(grammar_t *gmr, uint16_t type) {
    js_token_t *ret = next(gmr);
    if (ret->type != type) {
        printf("SyntaxError: Encountered %d, but %d expected\n", ret->type, type);
        assert(0);
    }
    return ret;
}

static bool expectSemicolon(grammar_t *gmr) {
    gmr->lex->regexp = true;
    js_token_t *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
    if (nxt->type == SEMICOLON) {
        next(gmr);
        return true;
    }
    if (nxt->type == R_BRACE || nxt->lineBefore) {
        return true;
    } else {
        assert(!"SyntaxError: Expected semicolon after statement.");
        return false;
    }
}

grammar_t *grammar_new(lex_t *lex) {
    grammar_t *gmr = malloc(sizeof(struct struct_grammar));
    gmr->lex = lex;
    gmr->next = NULL;
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
js_data_t *grammar_primaryExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    switch (lookahead(gmr)->type) {
        case THIS:
            next(gmr);
            return (js_data_t *)js_allocEmptyNode(THIS_NODE);
        case ID: assert(0); /*{
            js_token_t *id = next(gmr);
            primary_expr_id_node_t *node = (primary_expr_id_node_t *)createNode(PRIMARY_EXPR_ID_NODE, sizeof(literal_node_t));
            moveString(&node->name, &id->stringValue);
            lex_disposeToken(id);
            return (node_t *)node;
        }*/
        case NULL_LIT: {
            next(gmr);
            return js_constNull;
        }
        case TRUE_LIT: {
            next(gmr);
            return js_constTrue;
        }
        case FALSE_LIT: {
            next(gmr);
            return js_constFalse;
        }
        case NUM: {
            js_token_t *num = next(gmr);
            return num->value;
        }
        case STR: {
            js_token_t *str = next(gmr);
            return str->value;
        }
        case REGEXP: {
            assert(0);
        }

        case L_PAREN: {
            next(gmr);
            js_data_t *ret = grammar_expr(gmr);
            expect(gmr, R_PAREN);
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
js_data_t *grammar_memberExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    js_data_t *cur;
    switch (lookahead(gmr)->type) {
        case FUNCTION: {
            assert(!"FunctionLiteral not supported yet");
            break;
        }
        case NEW: {
            /*
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
            break;*/
            assert(0);
        }
        default:
            cur = grammar_primaryExpr(gmr);
            break;
    }
    gmr->lex->regexp = false;
    while (1) {
        switch (lookahead(gmr)->type) {
            case POINT: {
                next(gmr);
                gmr->lex->parseId = false;
                js_token_t *id = expect(gmr, ID);
                gmr->lex->parseId = true;
                js_binary_node_t *node = (js_binary_node_t *)js_allocBinaryNode(MEMBER_NODE);
                node->_1 = cur;
                node->_2 = id->value;
                cur = (js_data_t *)node;
                break;
            }
            case L_BRACKET: {
                next(gmr);
                js_data_t *expr = grammar_expr(gmr);
                expect(gmr, R_BRACKET);
                js_binary_node_t *node = (js_binary_node_t *)js_allocBinaryNode(MEMBER_NODE);
                node->_1 = cur;
                node->_2 = expr;
                cur = (js_data_t *)node;
                break;
            }
            default:
                return cur;
        }
    }
}

/*
 * Arguments := ( ArgumentList_opt )
 * /
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
}*/

/*
 * LeftHandSideExpression := MemberExpression
 *      (
            Arguments
            | . IdentifierName
            | [ Expression ]
 *      )*
 */
js_data_t *grammar_leftHandSideExpr(grammar_t *gmr) {
    js_data_t *cur = grammar_memberExpr(gmr);
    while (1) {
        switch (lookahead(gmr)->type) {
            /*case POINT: {
                discard(gmr);
                gmr->lex->parseId = false;
                js_token_t *id = expect(gmr, ID);
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
            }*/
            default:
                return cur;
        }
    }
}

js_data_t *grammar_postfixExpr(grammar_t *gmr) {
    js_data_t *expr = grammar_leftHandSideExpr(gmr);

    js_token_t *next = lookahead(gmr);
    if (next->lineBefore) {
        return expr;
    }

    /*if (next->type == INC) {
        unary_node_t *node =
            (unary_node_t *)createNode(POSTFIX_EXPR_INC_NODE, sizeof(unary_node_t));
        node->first = expr;
        return (node_t *)node;
    } else if (next->type == DEC) {
        unary_node_t *node =
            (unary_node_t *)createNode(POSTFIX_EXPR_DEC_NODE, sizeof(unary_node_t));
        node->first = expr;
        return (node_t *)node;
    }*/

    return expr;
}

js_data_t *grammar_unaryExpr(grammar_t *gmr) {
    gmr->lex->regexp = true;
    uint16_t nodeClass;
    js_token_t *lh = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (lh->type) {
        case DELETE:
            nodeClass = DELETE_NODE;
            goto produceExpr;
        case VOID:
            nodeClass = VOID_NODE;
            goto produceExpr;
        case TYPEOF:
            nodeClass = TYPEOF_NODE;
            goto produceExpr;
        case INC:
            nodeClass = INC_NODE;
            goto produceExpr;
        case DEC:
            nodeClass = DEC_NODE;
            goto produceExpr;
        case ADD:
            nodeClass = POS_NODE;
            goto produceExpr;
        case SUB:
            nodeClass = NEG_NODE;
            goto produceExpr;
        case NOT:
            nodeClass = NOT_NODE;
            goto produceExpr;
        case L_NOT:
            nodeClass = LNOT_NODE;
produceExpr:
            next(gmr);
            js_unary_node_t *node = js_allocUnaryNode(nodeClass);
            node->_1 = grammar_unaryExpr(gmr);
            return (js_data_t *)node;
        default:
            return grammar_postfixExpr(gmr);
    }
}

#define BINARY_HEAD(_name, _previous) js_data_t *grammar_##_name(grammar_t *gmr) {\
        js_data_t *cur = grammar_##_previous(gmr);\
        while (true) {\
            uint16_t type;\
            switch (lookahead(gmr)->type) {

#define BINARY_CASE(_value) case _value: {\
    type = _value##_NODE;\
    break;\
}\
 
#define BINARY_FOOT(_previous) default:\
    return cur;\
    }\
    next(gmr);\
    js_binary_node_t *node = js_allocBinaryNode(type);\
    node->_1 = cur;\
    node->_2 = grammar_##_previous(gmr);\
    cur = (js_data_t *)node;\
    }\
    }

#define BINARY_1(_name, _previous, _first) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_FOOT(_previous)

#define BINARY_2(_name, _previous, _first, _second) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_CASE(_second)\
    BINARY_FOOT(_previous)

#define BINARY_3(_name, _previous, _first, _second, _third) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_CASE(_second)\
    BINARY_CASE(_third)\
    BINARY_FOOT(_previous)

BINARY_3(mulExpr, unaryExpr, MUL, MOD, DIV);
BINARY_2(addExpr, mulExpr, ADD, SUB);
BINARY_3(shiftExpr, addExpr, SHL, SHR, USHR);

BINARY_HEAD(relExpr, shiftExpr)
BINARY_CASE(LT)
BINARY_CASE(GT)
BINARY_CASE(LTEQ)
BINARY_CASE(GTEQ)
BINARY_CASE(INSTANCEOF)
case IN: {
    if (gmr->noIn) {
        return cur;
    }
    type = IN_NODE;
    break;
}
BINARY_FOOT(shiftExpr);

BINARY_HEAD(eqExpr, relExpr)
BINARY_CASE(EQ)
BINARY_CASE(INEQ)
BINARY_CASE(FULL_EQ)
BINARY_CASE(FULL_INEQ)
BINARY_FOOT(relExpr)

BINARY_1(andExpr, eqExpr, AND)
BINARY_1(xorExpr, andExpr, XOR)
BINARY_1(orExpr, xorExpr, OR)
BINARY_1(lAndExpr, orExpr, L_AND)
BINARY_1(lOrExpr, lAndExpr, L_OR)

static js_data_t *grammar_condExpr(grammar_t *gmr) {
    js_data_t *node = grammar_lOrExpr(gmr);
    if (lookahead(gmr)->type == QUESTION) {
        next(gmr);
        js_data_t *t_exp = grammar_assignExpr(gmr);
        expect(gmr, COLON);
        js_data_t *f_exp = grammar_assignExpr(gmr);
        js_ternary_node_t *ret = js_allocTernaryNode(COND_NODE);
        ret->_1 = node;
        ret->_2 = t_exp;
        ret->_3 = f_exp;
        return (js_data_t *)ret;
    } else {
        return node;
    }
}

js_data_t *grammar_assignExpr(grammar_t *gmr) {
    js_data_t *node = grammar_condExpr(gmr);
    /* We do not care wether it is LeftHandSide. We can check it later */
    uint16_t type;
    switch (lookahead(gmr)->type) {
        case ASSIGN: type = ASSIGN_NODE; break;
        case MUL_ASSIGN: type = MUL_ASSIGN_NODE; break;
        case DIV_ASSIGN: type = DIV_ASSIGN_NODE; break;
        case MOD_ASSIGN: type = MOD_ASSIGN_NODE; break;
        case ADD_ASSIGN: type = ADD_ASSIGN_NODE; break;
        case SUB_ASSIGN: type = SUB_ASSIGN_NODE; break;
        case SHL_ASSIGN: type = SHL_ASSIGN_NODE; break;
        case SHR_ASSIGN: type = SHR_ASSIGN_NODE; break;
        case USHR_ASSIGN: type = USHR_ASSIGN_NODE; break;
        case AND_ASSIGN: type = AND_ASSIGN_NODE; break;
        case XOR_ASSIGN: type = XOR_ASSIGN_NODE; break;
        case OR_ASSIGN: type = OR_ASSIGN_NODE; break;
        default: return node;
    }
    next(gmr);
    js_binary_node_t *ass = js_allocBinaryNode(type);
    ass->_1 = node;
    ass->_2 = grammar_assignExpr(gmr);
    return (js_data_t *)ass;
}

BINARY_1(expr, assignExpr, COMMA);

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
* /
node_t *grammar_stmt(grammar_t *gmr) {
    gmr->lex->regexp = true;
    js_token_t *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (nxt->type) {
        case SEMICOLON:
            next(gmr);
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
 * /
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
        js_token_t *id = next(gmr);
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
 * /
static node_t *grammar_formalParamList(grammar_t *gmr) {
    js_token_t *id = expect(gmr, ID);

    var_node_t *var = (var_node_t *)createNode(VAR_NODE, sizeof(var_node_t));
    moveString(&var->name, &id->stringValue);
    lex_disposeToken(id);
    var_node_t *last = var;

    while (lookahead(gmr)->type == COMMA) {
        js_token_t *nextId = expect(gmr, ID);
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
    js_token_t *next = lookahead(gmr);
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
        js_token_t *next = lookahead(gmr);
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
    js_token_t *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == FUNCTION) {
        return grammar_funcDecl(gmr);
    } else {
        return grammar_stmt(gmr);
    }
}*/
