/**
 * Implements a in-memory file system
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdlib.h"
#include "c/string.h"
#include "c/assert.h"

#include "util/alignment.h"

#include "vfs.h"

static fs_node_t *ramfs_create(fs_node_t *parent, char *name);
static fs_node_t *ramfs_mkdir(fs_node_t *parent, char *name);
static fs_node_t *ramfs_readdir(fs_node_t *parent, uint32_t index);
static fs_node_t *ramfs_finddir(fs_node_t *parent, char *name);

static fs_op_t ramfs_op = {
    .read = NULL,
    .write = NULL,
    .readdir = ramfs_readdir,
    .finddir = NULL,
    .create = ramfs_create,
    .mkdir = ramfs_mkdir
};

typedef struct struct_ramfs_data_t {
    union {
        fs_node_t **dir;
        char *data;
    };
    uint64_t allocSize;
} ramfs_data_t;

static void ensureSize(fs_node_t *node, uint64_t size) {
    ramfs_data_t *data = node->dataPtr;
    if (size > data->allocSize) {
        data->allocSize = alignTo(size, 512);
        data->data = realloc(data->data, data->allocSize);
    }
}

static fs_node_t *ramfs_readdir(fs_node_t *parent, uint32_t index) {
    assert(parent->type == DIR);
    ramfs_data_t *node = parent->dataPtr;
    if (index * sizeof(size_t) >= parent->length) {
        return NULL;
    }
    return node->dir[index];
}

static fs_node_t *ramfs_createNode(fs_node_t *parent, char *name, uint8_t type) {
    fs_node_t *ret = malloc(sizeof(fs_node_t));
    ret->name = strdup(name);
    ret->op = &ramfs_op;
    ret->pointer = NULL;
    ret->length = 0;
    ret->type = type;

    ramfs_data_t *data = malloc(sizeof(ramfs_data_t));
    data->allocSize = 128;
    data->data = malloc(128);
    ret->dataPtr = data;

    if (!parent) {
        return ret;
    }

    ramfs_data_t *node = parent->dataPtr;
    assert(parent->type == DIR);
    ensureSize(parent, parent->length + sizeof(size_t));
    node->dir[parent->length / sizeof(size_t)] = ret;
    parent->length += sizeof(size_t);
    return ret;
}

static fs_node_t *ramfs_create(fs_node_t *parent, char *name) {
    return ramfs_createNode(parent, name, FILE);
}

static fs_node_t *ramfs_mkdir(fs_node_t *parent, char *name) {
    return ramfs_createNode(parent, name, DIR);
}

fs_node_t *ramfs_create_fs(void) {
    return ramfs_mkdir(NULL, "");
}