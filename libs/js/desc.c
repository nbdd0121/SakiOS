#include "js/type.h"

#include "c/stddef.h"
#include "c/stdlib.h"
#include "c/assert.h"

js_property_t *js_allocPropertyDesc(void) {
    js_property_t *prop = (js_property_t *)js_alloc(JS_INTERNAL_PROPERTY);
    prop->value = NULL;
    prop->get = NULL;
    prop->set = NULL;
    prop->writable = NULL;
    prop->configurable = NULL;
    prop->enumerable = NULL;
    return prop;
}

bool js_isAccessorDescriptor(js_property_t *desc) {
    if (!desc) {
        return false;
    }
    if (!desc->get && !desc->get) {
        // Writable should be present
        return false;
    }
    return true;
}

bool js_isDataDescriptor(js_property_t *desc) {
    if (!desc) {
        return false;
    }
    if (!desc->value && !desc->writable) {
        return false;
    }
    return true;
}

bool js_isGenericDescriptor(js_property_t *desc) {
    if (!desc) {
        return false;
    }
    if (!js_isDataDescriptor(desc) && !js_isAccessorDescriptor(desc)) {
        return true;
    }
    return false;
}