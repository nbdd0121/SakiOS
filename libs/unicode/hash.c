
#include "unicode/convert.h"

#include "unicode/hash.h"

#include "c/stdlib.h"
#include "c/stdint.h"

int utf16_comparator(void *a, void *b) {
    utf16_string_t *a1 = a, *b1 = b;
    if (a1->len - b1->len) {
        return a1->len - b1->len;
    }
    return memcmp(a1->str, b1->str, a1->len);
}

int utf16_hash(void *key) {
    utf16_string_t *c = key;
    int32_t h = 0;
    for (int i = 0; i < c->len; i++) {
        h = 31 * h + c->str[i];
    }
    return h;
}

hashmap_t *hashmap_new_utf16(int size) {
    return hashmap_new(utf16_hash, utf16_comparator, size);
}