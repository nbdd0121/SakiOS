/**
 * Header file for page manager
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stddef.h"

typedef struct struct_pageman_t pageman_t;
pageman_t *pageman_create(void *base, size_t limit, void *firstAval, size_t firstSize);
void pageman_free(pageman_t *bpm, void *addr, size_t size);
void *pageman_alloc(pageman_t *bpm, size_t size);
void pageman_freeBlock(pageman_t *bpm, void *addr, size_t size);
size_t pageman_spare(pageman_t *bpm);