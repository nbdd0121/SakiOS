
#include "unicode/convert.h"
#include "js/node.h"
#include "c/stdlib.h"
#include "c/assert.h"

void examine_node(node_t *node) {
    char *nodeName;
    switch (node->type) {
        case PRIMARY_EXPR_THIS_NODE: {
            printf("[this]");
            break;
        }
        case PRIMARY_EXPR_ID_NODE: {
            primary_expr_id_node_t *n = (primary_expr_id_node_t *)node;
            printf("[id, ");
            unicode_putUtf16(n->name);
            printf("]");
            break;
        }
        case LITERAL_NODE: {
            literal_node_t *n = (literal_node_t *)node;
            switch (n->value.type) {
                case SAKI_TYPE_NULL:
                    printf("[literal, null]");
                    break;
                case SAKI_TYPE_BOOLEAN:
                    printf("[literal, boolean, %s]", n->value.booleanValue ? "true" : "false");
                    break;
                case SAKI_TYPE_NUMBER:
                    printf("[literal, number, %d]", (size_t)n->value.numberValue);
                    break;
                case SAKI_TYPE_STRING:
                    printf("[literal, string, ");
                    unicode_putUtf16(n->value.stringValue);
                    printf("]");
                    break;
                default:
                    assert(0);
            }
            break;
        }
        case MEMBER_EXPR_NEW_NODE: {
            member_expr_new_node_t *n = (member_expr_new_node_t *)node;
            printf("[new, ");
            examine_node(n->expr);
            node_t *cur = n->args;
            while (cur) {
                printf(", ");
                examine_node(cur);
                cur = cur->next;
            }
            printf("]");
            break;
        }
        case MEMBER_EXPR_DOT_NODE: {
            member_expr_dot_node_t *n = (member_expr_dot_node_t *)node;
            printf("[dot, ");
            examine_node(n->expr);
            printf(", ");
            unicode_putUtf16(n->name);
            printf("]");
            break;
        }
        case MEMBER_EXPR_BRACKET_NODE: {
            member_expr_bracket_node_t *n = (member_expr_bracket_node_t *)node;
            printf("[bracket, ");
            examine_node(n->expr);
            printf(", ");
            examine_node(n->name);
            printf("]");
            break;
        }
        case CALL_EXPR_NODE: {
            call_expr_node_t *n = (call_expr_node_t *)node;
            printf("[call, ");
            examine_node(n->expr);
            node_t *cur = n->args;
            while (cur) {
                printf(", ");
                examine_node(cur);
                cur = cur->next;
            }
            printf("]");
            break;
        }
        case POSTFIX_EXPR_INC_NODE: nodeName = "post++"; goto examineUnary;
        case POSTFIX_EXPR_DEC_NODE:  nodeName = "post--"; goto examineUnary;
        case UNARY_EXPR_DELETE_NODE: nodeName = "delete"; goto examineUnary;
        case UNARY_EXPR_VOID_NODE: nodeName = "void"; goto examineUnary;
        case UNARY_EXPR_TYPEOF_NODE: nodeName = "typeof"; goto examineUnary;
        case UNARY_EXPR_INC_NODE: nodeName = "pre++"; goto examineUnary;
        case UNARY_EXPR_DEC_NODE: nodeName = "pre--"; goto examineUnary;
        case UNARY_EXPR_POS_NODE: nodeName = "pos"; goto examineUnary;
        case UNARY_EXPR_NEG_NODE: nodeName = "neg"; goto examineUnary;
        case UNARY_EXPR_NOT_NODE: nodeName = "~"; goto examineUnary;
        case UNARY_EXPR_LNOT_NODE: nodeName = "!"; goto examineUnary;
        case EXPR_STMT_NODE: nodeName = "ExprStmt"; goto examineUnary;
examineUnary: {
                unary_node_t *n = (unary_node_t *)node;
                printf("[%s, ", nodeName);
                examine_node(n->first);
                printf("]");
                break;
            }
        case MUL_EXPR_MUL_NODE: nodeName = "*"; goto examineBinary;
        case MUL_EXPR_DIV_NODE: nodeName = "/"; goto examineBinary;
        case MUL_EXPR_MOD_NODE: nodeName = "%"; goto examineBinary;
        case ADD_EXPR_ADD_NODE: nodeName = "+"; goto examineBinary;
        case ADD_EXPR_SUB_NODE: nodeName = "-"; goto examineBinary;
        case SHIFT_EXPR_SHL_NODE: nodeName = "<<"; goto examineBinary;
        case SHIFT_EXPR_SHR_NODE: nodeName = ">>"; goto examineBinary;
        case SHIFT_EXPR_USHR_NODE: nodeName = ">>>"; goto examineBinary;
        case REL_EXPR_LT_NODE: nodeName = "<"; goto examineBinary;
        case REL_EXPR_GT_NODE: nodeName = ">"; goto examineBinary;
        case REL_EXPR_LTEQ_NODE: nodeName = "<="; goto examineBinary;
        case REL_EXPR_GTEQ_NODE: nodeName = ">="; goto examineBinary;
        case REL_EXPR_INSTANCEOF_NODE: nodeName = "instanceof"; goto examineBinary;
        case REL_EXPR_IN_NODE: nodeName = "in"; goto examineBinary;
        case EQ_EXPR_EQ_NODE: nodeName = "=="; goto examineBinary;
        case EQ_EXPR_INEQ_NODE: nodeName = "!="; goto examineBinary;
        case EQ_EXPR_FULL_EQ_NODE: nodeName = "==="; goto examineBinary;
        case EQ_EXPR_FULL_INEQ_NODE: nodeName = "!=="; goto examineBinary;
        case BITWISE_EXPR_AND_NODE: nodeName = "&"; goto examineBinary;
        case BITWISE_EXPR_XOR_NODE: nodeName = "^"; goto examineBinary;
        case BITWISE_EXPR_OR_NODE: nodeName = "|"; goto examineBinary;
        case LOGICAL_EXPR_L_AND_NODE: nodeName = "&&"; goto examineBinary;
        case LOGICAL_EXPR_L_OR_NODE: nodeName = "||"; goto examineBinary;
        case ASSIGN_EXPR_ASSIGN_NODE: nodeName = "="; goto examineBinary;
        case ASSIGN_EXPR_MUL_NODE: nodeName = "*="; goto examineBinary;
        case ASSIGN_EXPR_DIV_NODE: nodeName = "/="; goto examineBinary;
        case ASSIGN_EXPR_MOD_NODE: nodeName = "%="; goto examineBinary;
        case ASSIGN_EXPR_ADD_NODE: nodeName = "+="; goto examineBinary;
        case ASSIGN_EXPR_SUB_NODE: nodeName = "-="; goto examineBinary;
        case ASSIGN_EXPR_SHL_NODE: nodeName = "<<="; goto examineBinary;
        case ASSIGN_EXPR_SHR_NODE: nodeName = ">>="; goto examineBinary;
        case ASSIGN_EXPR_USHR_NODE: nodeName = ">>>="; goto examineBinary;
        case ASSIGN_EXPR_AND_NODE: nodeName = "&="; goto examineBinary;
        case ASSIGN_EXPR_XOR_NODE: nodeName = "^="; goto examineBinary;
        case ASSIGN_EXPR_OR_NODE: nodeName = "|="; goto examineBinary;
        case EXPR_COMMA_NODE: nodeName = "','"; goto examineBinary;
examineBinary: {
                binary_node_t *n = (binary_node_t *)node;
                printf("[%s, ", nodeName);
                examine_node(n->first);
                printf(", ");
                examine_node(n->second);
                printf("]");
                break;
            }

        case COND_EXPR_NODE: {
            cond_node_t *n = (cond_node_t *)node;
            printf("[?, ");
            examine_node(n->expr);
            printf(", ");
            examine_node(n->true_expr);
            printf(", ");
            examine_node(n->false_expr);
            printf("]");
            break;
        }
        case FUNCTION_NODE: {
            function_node_t *n = (function_node_t *)node;
            printf("[function, ");
            if (n->name.len == 0) {
                printf("<anonymous>");
            } else {
                unicode_putUtf16(n->name);
            }
            node_t *args = n->args;
            while (args) {
                printf(", ");
                examine_node(args);
                args = args->next;
            }
            args = n->body;
            while (args) {
                printf(", ");
                examine_node(args);
                args = args->next;
            }
            printf("]");
            break;
        }
        case VAR_NODE: {
            var_node_t *n = (var_node_t *)node;
            printf("[var, ");
            unicode_putUtf16(n->name);
            printf("]");
            break;
        }
        default:
            printf("[%d]", node->type);
            assert(0);
    }
}
/*
void *evalNode(node_t *node) {
    switch (node->type) {
        default:
            printf("[%d]", node->type);
            assert(0);
            return NULL;
    }
}

void* eval_program(node_t *node) {

}*/