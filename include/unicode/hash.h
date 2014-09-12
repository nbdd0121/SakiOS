
#include "data-struct/hashmap.h"

int utf16_comparator(void *a, void *b);
int utf16_hash(void *key);
hashmap_t *hashmap_new_utf16(int size);