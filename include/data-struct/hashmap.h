#ifndef DATA_STRUCT_HASHMAP_H
#define DATA_STRUCT_HASHMAP_H

#include "c/stdbool.h"

typedef int (*comparator_t)(void *, void *);
typedef int (*hash_t)(void *);
typedef struct str_hashmap hashmap_t;
typedef struct {
    void *first;
    void *second;
} pair_t;

int string_hash(void *);
int string_comparator(void *, void *);
hashmap_t *hashmap_new_string(int size);
hashmap_t *hashmap_new(hash_t, comparator_t, int);
bool hashmap_put(hashmap_t *, void *, void *);
void *hashmap_get(hashmap_t *, void *);
void *hashmap_remove(hashmap_t *, void *);
void hashmap_dispose(hashmap_t *);
pair_t *hashmap_iterator(hashmap_t *hm);
pair_t *hashmap_next(pair_t *it);

#endif