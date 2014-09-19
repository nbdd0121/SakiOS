#include "js/type.h"
#include "c/assert.h"

js_reference_t *js_allocReference(js_data_t *base, js_string_t *refName, bool strict) {
    js_reference_t *ref = (js_reference_t *)js_alloc(JS_INTERNAL_REF);
    ref->base = base;
    ref->ref = refName;
    ref->strict = strict;
    return ref;
}

js_data_t *js_getBase(js_reference_t *ref) {
    return ref->base;
}

js_string_t *js_getReferencedName(js_reference_t *ref) {
    return ref->ref;
}

bool js_isStrictReference(js_reference_t *ref) {
    return ref->strict;
}

bool js_hasPrimitiveBase(js_reference_t *ref) {
    if (ref->base->type == JS_STRING || ref->base->type == JS_BOOLEAN || ref->base->type == JS_NUMBER) {
        return true;
    } else {
        return false;
    }
}

bool js_isPropertyReference(js_reference_t *ref) {
    if (js_hasPrimitiveBase(ref) || ref->base->type == JS_OBJECT) {
        return true;
    } else {
        return false;
    }
}

bool js_isUnresolvableReference(js_reference_t *ref) {
    if (ref->base->type == JS_UNDEFINED) {
        return true;
    } else {
        return false;
    }
}

js_data_t *js_getValue(js_data_t *arg0) {
    if (arg0->type != JS_INTERNAL_REF) {
        return arg0;
    }
    js_reference_t *V = (js_reference_t *)arg0;
    js_data_t *base = js_getBase(V);
    if (js_isUnresolvableReference(V)) {
        assert(!"ReferenceError");
    }
    if (js_isPropertyReference(V)) {
        if (js_hasPrimitiveBase(V)) {
            assert(0);
        } else {
            js_object_t *objBase = (js_object_t *)base;
            js_data_t *ret = objBase->get(objBase, js_getReferencedName(V));
            return ret;
        }
    } else {
        assert(0);
    }
}

void js_putValue(js_data_t *arg0, js_data_t *W) {
    if (arg0->type != JS_INTERNAL_REF) {
        assert(!"ReferenceError");
    }
    js_reference_t *V = (js_reference_t *)arg0;
    js_data_t *base = js_getBase(V);
    if (js_isUnresolvableReference(V)) {
        if (js_isStrictReference(V)) {
            assert(!"ReferenceError");
        } else {
            //Put global
            assert(!"NO GLOBAL");
        }
    } else if (js_isPropertyReference(V)) {
        if (js_hasPrimitiveBase(V)) {
            assert(0);
        } else {
            js_object_t *objBase = (js_object_t *)base;
            objBase->put(objBase, js_getReferencedName(V), W, js_isStrictReference(V));
            return;
        }
    } else {
        assert(0);
    }
}