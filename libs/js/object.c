
#include "js/type.h"

#include "c/stddef.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/convert.h"
#include "unicode/type.h"
#include "unicode/hash.h"

#include "data-struct/hashmap.h"

static int hash_js_string(void *str) {
    return unicode_utf16Hash(&((js_string_t *)str)->value);
}

static int compare_js_string(void *x, void *y) {
    return unicode_utf16Cmp(&((js_string_t *)x)->value, &((js_string_t *)y)->value);
}

static js_property_t *getOwnProperty(js_object_t *O, js_string_t *P) {
    js_property_t *desc = hashmap_get(O->properties, P);
    return desc;
}

static js_property_t *getProperty(js_object_t *O, js_string_t *P) {
    js_property_t *prop = O->getOwnProperty(O, P);
    if (prop) {
        return prop;
    }
    js_object_t *proto = O->prototype;
    if (!proto) {
        return NULL;
    }
    return proto->getOwnProperty(proto, P);
}

static js_data_t *get(js_object_t *O, js_string_t *P) {
    js_property_t *desc = O->getProperty(O, P);
    if (!desc) {
        return js_constUndefined;
    }
    if (js_isDataDescriptor(desc)) {
        return desc->value;
    } else {
        assert(!"GETTER NOT SUPPORTED");
        // js_object_t* getter=desc->get;
        // if(!getter) {
        //  return NULL;
        // }
        // return O.[[Call]](this)
    }
}

static bool canPut(js_object_t *O, js_string_t *P) {
    js_property_t *desc = O->getOwnProperty(O, P);
    if (desc) {
        if (js_isAccessorDescriptor(desc)) {
            if (desc->set) {
                return false;
            } else {
                return true;
            }
        } else {
            return js_isTrue(desc->writable);
        }
    }
    js_object_t *proto = O->prototype;
    if (!proto) {
        return O->extensible;
    }
    js_property_t *inherited = proto->getProperty(proto, P);
    if (!inherited) {
        return O->extensible;
    }
    if (js_isAccessorDescriptor(inherited)) {
        if (!inherited->set) {
            return false;
        } else {
            return true;
        }
    } else {
        if (!O->extensible) {
            return false;
        } else {
            return js_isTrue(inherited->writable);
        }
    }
}

static void put(js_object_t *O, js_string_t *P, js_data_t *V, bool throw) {
    if (!O->canPut(O, P)) {
        if (throw) {
            assert(!"TypeError");
        } else {
            return;
        }
    }
    js_property_t *ownDesc = O->getOwnProperty(O, P);
    if (js_isDataDescriptor(ownDesc)) {
        js_property_t *valueDesc = js_allocPropertyDesc();
        valueDesc->value = V;
        O->defineOwnProperty(O, P, valueDesc, throw);
        return;
    }
    js_property_t *desc = O->getProperty(O, P);
    if (js_isAccessorDescriptor(desc)) {
        //js_object_t *setter = desc->set;
        // Call Setter.[[Call]](this, V)
        assert(0);
    } else {
        js_property_t *newDesc = js_allocPropertyDesc();
        newDesc->value = V;
        newDesc->writable = js_constTrue;
        newDesc->enumerable = js_constTrue;
        newDesc->configurable = js_constTrue;
        O->defineOwnProperty(O, P, newDesc, throw);
        return;
    }
}

static bool hasProperty(js_object_t *O, js_string_t *P) {
    js_property_t *desc = O->getProperty(O, P);
    if (!desc) {
        return false;
    } else {
        return true;
    }
}

static bool _delete(js_object_t *O, js_string_t *P, bool throw) {
    js_property_t *desc = O->getOwnProperty(O, P);
    if (!desc) {
        return true;
    }
    if (js_isTrue(desc->configurable)) {
        hashmap_remove(O->properties, P);
        return true;
    } else {
        if (throw) {
            assert(!"TypeError");
        } else {
            return false;
        }
    }
}

static js_data_t *defaultValue(js_object_t *O, enum js_data_type_t hint) {
    assert(0);
}

static bool defineOwnProperty(js_object_t *O, js_string_t *P, js_property_t *desc, bool throw) {
    js_property_t *current = O->getOwnProperty(O, P);
    if (!current && !O->extensible) {
        if (throw) {
            assert(!"TypeError");
        } else {
            return false;
        }
    }
    if (!current && O->extensible) {
        if (js_isGenericDescriptor(desc) || js_isDataDescriptor(desc)) {
            if (!desc->value)desc->value = js_constUndefined;
            if (!desc->writable)desc->writable = js_constFalse;
            if (!desc->enumerable)desc->enumerable = js_constFalse;
            if (!desc->configurable)desc->configurable = js_constFalse;
            hashmap_put(O->properties, P, desc);
        } else {
            if (!desc->get)desc->get = js_constUndefined;
            if (!desc->set)desc->set = js_constUndefined;
            if (!desc->enumerable)desc->enumerable = js_constFalse;
            if (!desc->configurable)desc->configurable = js_constFalse;
            hashmap_put(O->properties, P, desc);
        }
        return true;
    }
    if (!desc->value && !desc->get && !desc->set && !desc->writable && !desc->enumerable && !desc->configurable) {
        return true;
    }
    //SameValue algorithm
    if (!js_isTrue(current->configurable)) {
        if (js_isTrue(desc->configurable)) {
            if (throw) {
                assert(!"TypeError");
            } else {
                return false;
            }
        } else if (desc->enumerable != current->enumerable) {
            if (throw) {
                assert(!"TypeError");
            } else {
                return false;
            }
        }
    }
    if (js_isGenericDescriptor(desc)) {

    } else {
        bool current_isDataDescriptor = js_isDataDescriptor(current);
        bool desc_isDataDescriptor = js_isDataDescriptor(desc);
        if (current_isDataDescriptor != desc_isDataDescriptor) {
            if (!js_isTrue(current->configurable)) {
                if (throw) {
                    assert(!"TypeError");
                } else {
                    return false;
                }
            }
            if (current_isDataDescriptor) {
                assert(0);
            } else {
                assert(0);
            }
        } else if (current_isDataDescriptor && desc_isDataDescriptor) {
            if (!js_isTrue(current->configurable)) {
                if (!js_isTrue(current->writable) && js_isTrue(desc->writable)) {
                    if (throw) {
                        assert(!"TypeError");
                    } else {
                        return false;
                    }
                } else if (!js_isTrue(current->writable)) {
                    assert(0);
                }
            }
        } else {
            if (!js_isTrue(current->configurable)) {
                assert(0);
            }
        }
    }
    if (desc->value)current->value = desc->value;
    if (desc->get)current->get = desc->get;
    if (desc->set)current->set = desc->set;
    if (desc->writable)current->writable = desc->writable;
    if (desc->enumerable)current->enumerable = desc->enumerable;
    if (desc->configurable)current->configurable = desc->configurable;
    return true;
}

js_object_t *js_allocObject(void) {
    js_object_t *obj = (js_object_t *)js_alloc(JS_OBJECT);
    obj->properties = hashmap_new(hash_js_string, compare_js_string, 11);
    obj->prototype = NULL;
    obj->clazz = NULL;
    obj->extensible = true;
    obj->getOwnProperty = getOwnProperty;
    obj->getProperty = getProperty;
    obj->get = get;
    obj->canPut = canPut;
    obj->put = put;
    obj->hasProperty = hasProperty;
    obj->_delete = _delete;
    obj->defaultValue = defaultValue;
    obj->defineOwnProperty = defineOwnProperty;
    return obj;
}
