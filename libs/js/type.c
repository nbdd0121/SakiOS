
#include "js/type.h"

#include "c/stddef.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/convert.h"

js_data_t *js_constNull = NULL;
js_data_t *js_constUndefined = NULL;
js_data_t *js_constTrue = NULL;
js_data_t *js_constFalse = NULL;
js_data_t *js_constNaN = NULL;
js_data_t *js_constZero = NULL;
js_data_t *js_constOne = NULL;
js_string_t *js_constNullStr = NULL;
js_string_t *js_constUndefStr = NULL;
js_string_t *js_constTrueStr = NULL;
js_string_t *js_constFalseStr = NULL;
js_string_t *js_constNaNStr = NULL;
js_string_t *js_constInfStr = NULL;
js_string_t *js_constNegInfStr = NULL;
js_string_t *js_constZeroStr = NULL;

js_data_t *js_alloc(enum js_data_type_t type) {
    size_t size;
    switch (type) {
        case JS_NULL:
        case JS_UNDEFINED:
        case JS_BOOLEAN:
            size = sizeof(js_data_t);
            break;
        case JS_STRING:
            size = sizeof(js_string_t);
            break;
        case JS_NUMBER:
            size = sizeof(js_number_t);
            break;
        case JS_OBJECT:
            size = sizeof(js_object_t);
            break;
        case JS_INTERNAL_REF:
            size = sizeof(js_reference_t);
            break;
        case JS_INTERNAL_PROPERTY:
            size = sizeof(js_property_t);
            break;
        case JS_INTERNAL_TOKEN:
            size = sizeof(js_token_t);
            break;
        case JS_INTERNAL_EMPTY_NODE:
            size = sizeof(js_empty_node_t);
            break;
        case JS_INTERNAL_UNARY_NODE:
            size = sizeof(js_unary_node_t);
            break;
        case JS_INTERNAL_BINARY_NODE:
            size = sizeof(js_binary_node_t);
            break;
        case JS_INTERNAL_TERNARY_NODE:
            size = sizeof(js_ternary_node_t);
            break;
        default:
            assert(0);
    }
    js_data_t *data = malloc(size);
    data->type = type;
    data->flag = 0;
    return data;
}

js_token_t *js_allocToken(enum js_token_type_t type) {
    js_token_t *token = (js_token_t *)js_alloc(JS_INTERNAL_TOKEN);
    token->type = type;
    token->next = NULL;
    token->value = NULL;
    token->lineBefore = false;
    return token;
}

js_empty_node_t *js_allocEmptyNode(enum js_empty_node_type_t type) {
    js_empty_node_t *node = (js_empty_node_t *)js_alloc(JS_INTERNAL_EMPTY_NODE);
    node->type = type;
    return node;
}

js_unary_node_t *js_allocUnaryNode(enum js_unary_node_type_t type) {
    js_unary_node_t *node = (js_unary_node_t *)js_alloc(JS_INTERNAL_UNARY_NODE);
    node->type = type;
    return node;
}

js_binary_node_t *js_allocBinaryNode(enum js_binary_node_type_t type) {
    js_binary_node_t *node = (js_binary_node_t *)js_alloc(JS_INTERNAL_BINARY_NODE);
    node->type = type;
    return node;
}

js_ternary_node_t *js_allocTernaryNode(enum js_ternary_node_type_t type) {
    js_ternary_node_t *node = (js_ternary_node_t *)js_alloc(JS_INTERNAL_TERNARY_NODE);
    node->type = type;
    return node;
}

#define js_const(val) ({typeof(val) _tmp=(val);((js_data_t*)_tmp)->flag = 1;_tmp;})

js_data_t *js_new_number(double value) {
    js_number_t *obj = (js_number_t *)js_alloc(JS_NUMBER);
    obj->value = value;
    return (js_data_t *)obj;
}

js_string_t *js_new_string(utf16_string_t str) {
    js_string_t *obj = (js_string_t *)js_alloc(JS_STRING);
    obj->value = str;
    return obj;
}

void js_init(void) {
    js_constNull = js_const(js_alloc(JS_NULL));
    js_constUndefined = js_const(js_alloc(JS_UNDEFINED));
    js_constTrue = js_const(js_alloc(JS_BOOLEAN));
    js_constFalse = js_const(js_alloc(JS_BOOLEAN));
    js_constNaN = js_const(js_new_number(0.0 / 0.0));
    js_constZero = js_const(js_new_number(0));
    js_constOne = js_const(js_new_number(1));
    js_constNullStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("null"))));
    js_constUndefStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("undefined"))));
    js_constTrueStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("true"))));
    js_constFalseStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("false"))));
    js_constNaNStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("NaN"))));
    js_constInfStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("Infinity"))));
    js_constNegInfStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("-Infinity"))));
    js_constZeroStr = js_const(js_new_string(unicode_toUtf16(UTF8_STRING("0"))));
}

