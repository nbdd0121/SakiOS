/**
 * Provide list functionality
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef DATA_STRUCT_LIST_H
#define DATA_STRUCT_LIST_H

#include "stdbool.h"

#define ENABLE_LIST(name) list_t name

typedef struct str_list_t {
    struct str_list_t *prev;
    struct str_list_t *next;
} list_t;

static inline void list_empty(list_t *list) {
    list->prev = list->next = list;
}

#define GET_LIST(str, name) (&(str)->name)
#define GET_DATA(list, str, name) (str*)((size_t)list - offsetof(str, name))

#define list_add list_addLast

static inline bool list_isEmpty(list_t *list) {
    return list->prev == list;
}

static inline void list_addFirst(list_t *list, list_t *inserted) {
    inserted->next = list->next;
    inserted->prev = list;
    list->next->prev = inserted;
    list->next = inserted;
}

static inline void list_addLast(list_t *list, list_t *inserted) {
    inserted->prev = list->prev;
    inserted->next = list;
    list->prev->next = inserted;
    list->prev = inserted;
}

static inline void list_remove(list_t *list) {
    list->prev->next = list->next;
    list->next->prev = list->prev;
}

#define list_forEach(list, var, str, name)\
    for (\
            list_t *__listVar = (list)->next, *__next = __listVar->next;\
            __listVar != list && (var = GET_DATA(__listVar, str, name));\
            __listVar = __next, __next = __listVar->next\
        )\
         
#endif
