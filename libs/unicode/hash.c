
#include "unicode/convert.h"

#include "unicode/hash.h"

#include "c/stdlib.h"
#include "c/stdint.h"

int unicode_utf16Cmp(void *x, void *y) {
    utf16_string_t *s1 = (utf16_string_t *)x, *s2 = (utf16_string_t *)y;
    int len = s1->len > s2->len ? s2->len : s1->len;
    for (int i = 0; i < len; i++) {
        int diff = s1->str[i] - s2->str[i];
        if (diff != 0) {
            return diff;
        }
    }
    return s1->len - s2->len;
}

int unicode_utf16Hash(void *key) {
    utf16_string_t *c = key;
    int32_t h = 0;
    for (int i = 0; i < c->len; i++) {
        h = 31 * h + c->str[i];
    }
    return h;
}

hashmap_t *hashmap_new_utf16(int size) {
    return hashmap_new(unicode_utf16Hash, unicode_utf16Cmp, size);
}