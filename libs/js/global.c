#include "js/type.h"

#include "c/stddef.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/convert.h"
#include "unicode/type.h"
#include "unicode/hash.h"

#include "data-struct/hashmap.h"

static js_property_t *createDesc(js_data_t *val) {
    js_property_t *prop = js_allocPropertyDesc();
    prop->writable = js_constTrue;
    prop->enumerable = js_constFalse;
    prop->configurable = js_constTrue;
    prop->value = val;
    return prop;
}

static js_property_t *createReadonlyDesc(js_data_t *val) {
    js_property_t *prop = js_allocPropertyDesc();
    prop->writable = js_constFalse;
    prop->enumerable = js_constFalse;
    prop->configurable = js_constFalse;
    prop->value = val;
    return prop;
}

js_object_t *js_createGlobal(void) {
    js_object_t *global = js_allocObject();
    global->extensible = true;
    global->prototype = NULL;
    global->clazz = js_constUndefStr;
    global->defineOwnProperty(global, js_constNaNStr, createReadonlyDesc(js_constNaN), true);
    global->defineOwnProperty(global, js_constInfStr, createReadonlyDesc(js_new_number(1.0 / 0.0)), true);
    global->defineOwnProperty(global, js_constUndefStr, createReadonlyDesc(js_constUndefined), true);
    return global;
}