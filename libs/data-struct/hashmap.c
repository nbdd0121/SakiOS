#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include "data-struct/hashmap.h"

typedef struct str_node {
    int hash;
    void *key;
    void *data;
    struct str_node *next;
} node_t;

struct str_hashmap {
    comparator_t compare;
    hash_t hash;
    int size;
    node_t *node[0];
};

typedef struct {
    pair_t kp;
    hashmap_t *hm;
    int index;
    node_t *next;
} iterator_t;

int string_comparator(void *a, void *b) {
    return strcmp(a, b);
}

int string_hash(void *key) {
    char *c = key;
    int32_t h = 0;
    while (*c != '\0') {
        h = 31 * h + *c++;
    }
    return h;
}

hashmap_t *hashmap_new_string(int size) {
    return hashmap_new(string_hash, string_comparator, size);
}

hashmap_t *hashmap_new(hash_t h, comparator_t c, int size) {
    hashmap_t *hm = malloc(sizeof(hashmap_t) + sizeof(node_t *)*size);
    //printf("%d",sizeof(hashmap_t)+sizeof(node_t*)*size);
    hm->compare = c;
    hm->hash = h;
    hm->size = size;
    int i;
    for (i = 0; i < size; i++) {
        hm->node[i] = NULL;
    }
    return hm;
}

bool hashmap_put(hashmap_t *hm, void *key, void *data) {
    int hash = hm->hash(key);
    int bucket = (unsigned int)hash % hm->size;
    node_t **n = &hm->node[bucket];
    while (*n != NULL) {
        node_t *c = *n;
        if (c->hash == hash && hm->compare(c->key, key) == 0) {
            c->data = data;
            return false;
        }
        n = &c->next;
    }
    *n = malloc(sizeof(node_t));
    node_t *mn = *n;
    mn->hash = hash;
    mn->key = key;
    mn->data = data;
    mn->next = NULL;
    return true;
}

void *hashmap_get(hashmap_t *hm, void *key) {
    int hash = hm->hash(key);
    int bucket = (unsigned int)hash % hm->size;
    node_t **n = &hm->node[bucket];
    while (*n != NULL) {
        node_t *c = *n;
        if (c->hash == hash && hm->compare(c->key, key) == 0) {
            return c->data;
        }
        n = &c->next;
    }
    return NULL;
}

void *hashmap_remove(hashmap_t *hm, void *key) {
    int hash = hm->hash(key);
    int bucket = (unsigned int)hash % hm->size;
    node_t **n = &hm->node[bucket];
    while (*n != NULL) {
        node_t *c = *n;
        if (c->hash == hash && hm->compare(c->key, key) == 0) {
            *n = c->next;
            void *data = c->data;
            free(c);
            return data;
        }
        n = &c->next;
    }
    return NULL;
}

void hashmap_dispose(hashmap_t *hm) {
    int i;
    for (i = 0; i < hm->size; i++) {
        node_t *mn = hm->node[i];
        while (mn != NULL) {
            node_t *m = mn;
            mn = m->next;
            free(m);
        }
    }
    free(hm);
}

pair_t *hashmap_iterator(hashmap_t *hm) {
    iterator_t *i = malloc(sizeof(iterator_t));
    i->kp.first = NULL;
    i->kp.second = NULL;
    i->hm = hm;
    i->index = -1;
    i->next = NULL;
    return &i->kp;
}

pair_t *hashmap_next(pair_t *it) {
    iterator_t *i = (iterator_t *)it;
    if (i->next) {
        i->kp.first = i->next->key;
        i->kp.second = i->next->data;
        i->next = i->next->next;
        return &i->kp;
    } else {
        for (i->index++; i->index < i->hm->size; i->index++) {
            node_t *mn = i->hm->node[i->index];
            if (mn != NULL) {
                i->kp.first = mn->key;
                i->kp.second = mn->data;
                i->next = mn->next;
                return &i->kp;
            }
        }
        free(i);
        return NULL;
    }
}
