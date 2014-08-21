#include "c/string.h"
#include "c/assert.h"
#include "c/stdbool.h"
#include "vfs.h"

fs_node_t *root;
fs_op_t empty = {
    .read = NULL,
    .write = NULL,
    .readdir = NULL,
    .finddir = NULL,
    .create = NULL,
    .mkdir = NULL
};

uint32_t vfs_read(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer) {
    if (node->pointer) {
        return vfs_read(node->pointer, offset, size, buffer);
    }
    if (node->op->read) {
        return node->op->read(node, offset, size, buffer);
    }
    return 0;
}

uint32_t vfs_write(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer) {
    if (node->pointer) {
        return vfs_write(node->pointer, offset, size, buffer);
    }
    if (node->op->write) {
        return node->op->write(node, offset, size, buffer);
    }
    return 0;
}

fs_node_t *vfs_readdir(fs_node_t *node, uint32_t index) {
    if (node->pointer) {
        return vfs_readdir(node->pointer, index);
    }
    if (node->op->readdir) {
        return node->op->readdir(node, index);
    }
    assert(0);
    return NULL;
}

fs_node_t *vfs_finddir(fs_node_t *node, char *name) {
    if (node->pointer) {
        return vfs_finddir(node->pointer, name);
    }
    if (node->op->finddir) {
        return node->op->finddir(node, name);
    }
    assert(0);
    return NULL;
}

fs_node_t *vfs_create(fs_node_t *node, char *name) {
    if (node->pointer) {
        return vfs_create(node->pointer, name);
    }
    if (node->op->create) {
        return node->op->create(node, name);
    }
    assert(0);
    return NULL;
}

fs_node_t *vfs_mkdir(fs_node_t *node, char *name) {
    if (node->pointer) {
        return vfs_mkdir(node->pointer, name);
    }
    if (node->op->mkdir) {
        return node->op->mkdir(node, name);
    }
    assert(0);
    return NULL;
}

static fs_node_t *lookup(fs_node_t *node, char *path) {
    if (path[0] == 0) {
        return node;
    }
    char *pathEnd;
    for (pathEnd = path; *pathEnd != 0; pathEnd++) {
        if (*pathEnd == '/') {
            char subPath[pathEnd - path + 1];
            memcpy(subPath, path, pathEnd - path);
            subPath[pathEnd - path] = 0;
            fs_node_t *cur = vfs_finddir(node, subPath);
            if (cur == NULL) {
                cur = vfs_mkdir(node, subPath);
            }
            return lookup(cur, pathEnd + 1);
        }
    }
    fs_node_t *ret = vfs_finddir(node, path);
    if (ret == NULL) {
        ret = vfs_create(node, path);
    }
    return vfs_finddir(node, path);
}

fs_node_t *vfs_lookup(char *path) {
    assert(path[0] == '/');
    return lookup(root, path + 1);
}

void vfs_mount(char *path, fs_node_t *node) {
    fs_node_t *p = vfs_lookup(path);
    assert(p && p->pointer == NULL);
    p->pointer = node;
}

void vfs_init(void) {
    root = malloc(sizeof(fs_node_t));
    root->name = strdup("");
    root->op = &empty;
    root->pointer = NULL;
    root->length = 0;
    root->type = DIR;
}