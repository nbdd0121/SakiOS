#include "js/type.h"

#include "c/assert.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/math.h"
#include "c/inttypes.h"

#include "unicode/hash.h"

js_data_t *js_evalEmptyNode(js_context_t *context, js_empty_node_t *node) {
    switch (node->type) {
        case THIS_NODE: {
            return (js_data_t *)context->thisBinding;
        }
        default:
            assert(0);
    }
}

js_data_t *js_evalUnaryNode(js_context_t *context, js_unary_node_t *node) {
    switch (node->type) {
        case POST_INC_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            // Eval&Arguments
            js_data_t *oldValue = js_toNumber(js_getValue(expr));
            js_data_t *newValue = js_new_number(((js_number_t *)oldValue)->value + 1.);
            js_putValue(expr, newValue);
            return oldValue;
        }
        case POST_DEC_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            // Eval&Arguments
            js_data_t *oldValue = js_toNumber(js_getValue(expr));
            js_data_t *newValue = js_new_number(((js_number_t *)oldValue)->value - 1.);
            js_putValue(expr, newValue);
            return oldValue;
        }
        case DELETE_NODE: {
            assert(0);
        }
        case VOID_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            js_getValue(expr);
            return js_constUndefined;
        }
        case TYPEOF_NODE: {
            assert(0);
        }
        case PRE_INC_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            // Eval&Arguments
            js_data_t *oldValue = js_toNumber(js_getValue(expr));
            js_data_t *newValue = js_new_number(((js_number_t *)oldValue)->value + 1.);
            js_putValue(expr, newValue);
            return newValue;
        }
        case PRE_DEC_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            // Eval&Arguments
            js_data_t *oldValue = js_toNumber(js_getValue(expr));
            js_data_t *newValue = js_new_number(((js_number_t *)oldValue)->value - 1.);
            js_putValue(expr, newValue);
            return newValue;
        }
        case POS_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            return js_toNumber(js_getValue(expr));
        }
        case NEG_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            js_number_t *oldValue = (js_number_t *)js_toNumber(js_getValue(expr));
            if (isnan(oldValue->value)) {
                return js_constNaN;
            }
            return js_new_number(-oldValue->value);
        }
        case NOT_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            int32_t oldValue = js_toInt32(js_getValue(expr));
            return js_new_number(~oldValue);
        }
        case LNOT_NODE: {
            js_data_t *expr = js_evalNode(context, node->_1);
            js_data_t *oldValue = js_toBoolean(js_getValue(expr));
            if (oldValue == js_constTrue) {
                return js_constFalse;
            } else {
                return js_constTrue;
            }
        }

        case EXPR_STMT: {
            js_data_t *exprRef = js_evalNode(context, node->_1);
            js_completion_t *comp = js_allocCompletion(COMPLETION_NORMAL);
            comp->value = js_getValue(exprRef);
            return (js_data_t *)comp;
        }
        default: assert(0);
    }
}

static js_data_t *absRelComp(js_data_t *x, js_data_t *y, bool leftFirst) {
    js_data_t *px, *py;
    if (leftFirst) {
        px = js_toPrimitive(x); //Hint Number
        py = js_toPrimitive(y); //Hint Number
    } else {
        py = js_toPrimitive(y); //Hint Number
        px = js_toPrimitive(x); //Hint Number
    }
    if (px->type != JS_STRING || py->type != JS_STRING) {
        js_number_t *nx = (js_number_t *)js_toNumber(px);
        js_number_t *ny = (js_number_t *)js_toNumber(py);
        if (isnan(nx->value) || isnan(ny->value)) {
            return js_constUndefined;
        }
        if (nx->value < ny->value) {
            return js_constTrue;
        } else {
            return js_constFalse;
        }
    } else {
        js_string_t *lstr = js_toString(px);
        js_string_t *rstr = js_toString(py);
        int result = unicode_utf16Cmp(&lstr->value, &rstr->value);
        if (result < 0) {
            return js_constTrue;
        } else {
            return js_constFalse;
        }
    }
}

static js_data_t *absEqComp(js_data_t *x, js_data_t *y) {
    if (x->type == y->type) {
        switch (x->type) {
            case JS_UNDEFINED:
            case JS_NULL:
                return js_constTrue;
            case JS_NUMBER:
                if (((js_number_t *)x)->value == ((js_number_t *)y)->value) {
                    return js_constTrue;
                } else {
                    return js_constFalse;
                }
            case JS_STRING:
                if (unicode_utf16Cmp(&((js_string_t *)x)->value, &((js_string_t *)y)->value) == 0) {
                    return js_constTrue;
                } else {
                    return js_constFalse;
                }
            case JS_BOOLEAN:
                if (x == y) {
                    return js_constTrue;
                } else {
                    return js_constFalse;
                }
            default:
                assert(0);
        }
    } else if (x->type == JS_NULL && y->type == JS_UNDEFINED) {
        return js_constTrue;
    } else if (x->type == JS_UNDEFINED && y->type == JS_NULL) {
        return js_constTrue;
    } else if (x->type == JS_NUMBER && y->type == JS_STRING) {
        return absEqComp(x, js_toNumber(y));
    } else if (x->type == JS_STRING && y->type == JS_NUMBER) {
        return absEqComp(js_toNumber(x), y);
    } else if (x->type == JS_BOOLEAN) {
        return absEqComp(js_toNumber(x), y);
    } else if (y->type == JS_BOOLEAN) {
        return absEqComp(x, js_toNumber(y));
    } else if ((x->type == JS_STRING || x->type == JS_NUMBER) && y->type == JS_OBJECT) {
        return absEqComp(x, js_toPrimitive(y));
    } else if ((y->type == JS_STRING || y->type == JS_NUMBER) && x->type == JS_OBJECT) {
        return absEqComp(js_toPrimitive(x), y);
    } else {
        return js_constFalse;
    }
}

static js_data_t *strictEqComp(js_data_t *x, js_data_t *y) {
    if (x->type != y->type) {
        return js_constFalse;
    }
    switch (x->type) {
        case JS_UNDEFINED:
        case JS_NULL:
            return js_constTrue;
        case JS_NUMBER:
            if (((js_number_t *)x)->value == ((js_number_t *)y)->value) {
                return js_constTrue;
            } else {
                return js_constFalse;
            }
        case JS_STRING:
            if (unicode_utf16Cmp(&((js_string_t *)x)->value, &((js_string_t *)y)->value) == 0) {
                return js_constTrue;
            } else {
                return js_constFalse;
            }
        case JS_BOOLEAN:
            if (x == y) {
                return js_constTrue;
            } else {
                return js_constFalse;
            }
        default:
            assert(0);
    }
}

static js_data_t *doBinaryOp(enum js_binary_node_type_t type, js_data_t *lval, js_data_t *rval) {
    switch (type) {
        case MUL_NODE: {
            js_number_t *leftNum = (js_number_t *)js_toNumber(lval);
            js_number_t *rightNum = (js_number_t *)js_toNumber(rval);
            return js_new_number(leftNum->value * rightNum->value);
        }
        case DIV_NODE: {
            js_number_t *leftNum = (js_number_t *)js_toNumber(lval);
            js_number_t *rightNum = (js_number_t *)js_toNumber(rval);
            return js_new_number(leftNum->value / rightNum->value);
        }
        case MOD_NODE: {
            js_number_t *leftNum = (js_number_t *)js_toNumber(lval);
            js_number_t *rightNum = (js_number_t *)js_toNumber(rval);
            return js_new_number(fmod(leftNum->value, rightNum->value));
        }
        case ADD_NODE: {
            js_data_t *lprim = js_toPrimitive(lval);
            js_data_t *rprim = js_toPrimitive(rval);
            if (lprim->type == JS_STRING || rprim->type == JS_STRING) {
                js_string_t *lstr = js_toString(lprim);
                js_string_t *rstr = js_toString(rprim);
                size_t len = lstr->value.len + rstr->value.len;
                uint16_t *str = malloc(len * sizeof(uint16_t));
                memcpy(str, lstr->value.str, lstr->value.len * sizeof(uint16_t));
                memcpy(str + lstr->value.len, rstr->value.str, rstr->value.len * sizeof(uint16_t));
                return (js_data_t *)js_new_string((utf16_string_t) {
                    .str = str, .len = len
                });
            } else {
                js_number_t *lnum = (js_number_t *)js_toNumber(lprim);
                js_number_t *rnum = (js_number_t *)js_toNumber(rprim);
                return js_new_number(lnum->value + rnum->value);
            }
        }
        case SUB_NODE: {
            js_number_t *leftNum = (js_number_t *)js_toNumber(lval);
            js_number_t *rightNum = (js_number_t *)js_toNumber(rval);
            return js_new_number(leftNum->value - rightNum->value);
        }
        case SHL_NODE: {
            int32_t lnum = js_toInt32(lval);
            uint32_t rnum = js_toUint32(rval);
            int32_t shiftCount = rnum & 0x1F;
            return js_new_number(lnum << shiftCount);
        }
        case SHR_NODE: {
            int32_t lnum = js_toInt32(lval);
            uint32_t rnum = js_toUint32(rval);
            int32_t shiftCount = rnum & 0x1F;
            return js_new_number(lnum >> shiftCount);
        }
        case USHR_NODE: {
            uint32_t lnum = js_toUint32(lval);
            uint32_t rnum = js_toUint32(rval);
            uint32_t shiftCount = rnum & 0x1F;
            return js_new_number(lnum >> shiftCount);
        }
        case AND_NODE: {
            int32_t lnum = js_toInt32(lval);
            int32_t rnum = js_toInt32(rval);
            return js_new_number(lnum & rnum);
        }
        case XOR_NODE: {
            int32_t lnum = js_toInt32(lval);
            int32_t rnum = js_toInt32(rval);
            return js_new_number(lnum ^ rnum);
        }
        case OR_NODE: {
            int32_t lnum = js_toInt32(lval);
            int32_t rnum = js_toInt32(rval);
            return js_new_number(lnum | rnum);
        }
        default:
            assert(0);
    }
}

js_data_t *js_evalBinaryNode(js_context_t *context, js_binary_node_t *node) {
    switch (node->type) {
        case MEMBER_NODE: {
            js_data_t *baseValue = js_getValue(js_evalNode(context, node->_1));
            js_data_t *propNameVal = js_getValue(js_evalNode(context, node->_2));
            js_checkObjectCoercible(baseValue);
            js_string_t *propNameStr = js_toString(propNameVal);
            // Strict?
            return (js_data_t *)js_allocReference(baseValue, propNameStr, false);
        }
        case MUL_NODE:
        case DIV_NODE:
        case MOD_NODE:
        case ADD_NODE:
        case SUB_NODE:
        case SHL_NODE:
        case SHR_NODE:
        case USHR_NODE:
        case AND_NODE:
        case XOR_NODE:
        case OR_NODE: {
            js_data_t *leftValue = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rightValue = js_getValue(js_evalNode(context, node->_2));
            return doBinaryOp(node->type, leftValue, rightValue);
        }
        case LT_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = absRelComp(lval, rval, true);
            if (r == js_constUndefined) {
                return js_constFalse;
            } else {
                return r;
            }
        }
        case GT_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = absRelComp(rval, lval, false);
            if (r == js_constUndefined) {
                return js_constFalse;
            } else {
                return r;
            }
        }
        case LTEQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = absRelComp(rval, lval, false);
            if (r != js_constFalse) {
                return js_constFalse;
            } else {
                return js_constTrue;
            }
        }
        case GTEQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = absRelComp(lval, rval, true);
            if (r != js_constFalse) {
                return js_constFalse;
            } else {
                return js_constTrue;
            }
        }
        case INSTANCEOF_NODE:
        case IN_NODE:
            assert(0);
        case EQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            return absEqComp(rval, lval);
        }
        case INEQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = absEqComp(rval, lval);
            if (r == js_constTrue) {
                return js_constFalse;
            } else {
                return js_constTrue;
            }
        }
        case FULL_EQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            return strictEqComp(rval, lval);
        }
        case FULL_INEQ_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            js_data_t *r = strictEqComp(rval, lval);
            if (r == js_constTrue) {
                return js_constFalse;
            } else {
                return js_constTrue;
            }
        }
        case L_AND_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            if (js_toBoolean(lval) == js_constFalse) {
                return lval;
            }
            return js_getValue(js_evalNode(context, node->_2));
        }
        case L_OR_NODE: {
            js_data_t *lval = js_getValue(js_evalNode(context, node->_1));
            if (js_toBoolean(lval) == js_constTrue) {
                return lval;
            }
            return js_getValue(js_evalNode(context, node->_2));
        }
        case ASSIGN_NODE: {
            js_data_t *lref = js_evalNode(context, node->_1);
            js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
            // Check whether it is assign to eval or arguments in strict mode
            js_putValue(lref, rval);
            return rval;
        }
        {
            enum js_binary_node_type_t opType;
            case MUL_ASSIGN_NODE: opType = MUL_NODE; goto doOp;
            case DIV_ASSIGN_NODE: opType = DIV_NODE; goto doOp;
            case MOD_ASSIGN_NODE: opType = MOD_NODE; goto doOp;
            case ADD_ASSIGN_NODE: opType = ADD_NODE; goto doOp;
            case SUB_ASSIGN_NODE: opType = SUB_NODE; goto doOp;
            case SHL_ASSIGN_NODE: opType = SHL_NODE; goto doOp;
            case SHR_ASSIGN_NODE: opType = SHR_NODE; goto doOp;
            case USHR_ASSIGN_NODE: opType = USHR_NODE; goto doOp;
            case AND_ASSIGN_NODE: opType = AND_NODE; goto doOp;
            case XOR_ASSIGN_NODE: opType = XOR_NODE; goto doOp;
            case OR_ASSIGN_NODE: opType = OR_NODE; goto doOp;
doOp: {
                    js_data_t *lref = js_evalNode(context, node->_1);
                    js_data_t *lval = js_getValue(lref);
                    js_data_t *rval = js_getValue(js_evalNode(context, node->_2));
                    js_data_t *r = doBinaryOp(opType, lval, rval);
                    // Check whether it is assign to eval or arguments in strict mode
                    js_putValue(lref, r);
                    return r;
                }
            }
        case COMMA_NODE: {
            js_getValue(js_evalNode(context, node->_1));
            return js_getValue(js_evalNode(context, node->_2));
        }
        default:
            assert(0);
    }
}

js_data_t *js_evalTernaryNode(js_context_t *context, js_ternary_node_t *node) {
    switch (node->type) {
        case COND_NODE: {
            if (js_toBoolean(js_getValue(js_evalNode(context, node->_1))) == js_constTrue) {
                return js_getValue(js_evalNode(context, node->_2));
            } else {
                return js_getValue(js_evalNode(context, node->_3));
            }
        }
        default:
            assert(0);
    }
}

js_data_t *js_evalNode(js_context_t *context, js_data_t *node) {
    switch (node->type) {
        case JS_UNDEFINED:
        case JS_NULL:
        case JS_BOOLEAN:
        case JS_NUMBER:
            return node;
        case JS_STRING:
            return node;
        case JS_INTERNAL_EMPTY_NODE:
            return js_evalEmptyNode(context, (js_empty_node_t *)node);
        case JS_INTERNAL_UNARY_NODE:
            return js_evalUnaryNode(context, (js_unary_node_t *)node);
        case JS_INTERNAL_BINARY_NODE:
            return js_evalBinaryNode(context, (js_binary_node_t *)node);
        case JS_INTERNAL_TERNARY_NODE:
            return js_evalTernaryNode(context, (js_ternary_node_t *)node);
        default:
            assert(0);
    }
}