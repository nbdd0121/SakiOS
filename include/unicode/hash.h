
#include "data-struct/hashmap.h"

int unicode_utf16Cmp(void *s1, void *s2);
int unicode_utf16Hash(void *key);
hashmap_t *hashmap_new_utf16(int size);