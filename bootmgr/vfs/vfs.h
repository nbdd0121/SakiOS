/**
 * Defines data structures used by VFS.
 * This is a very tiny VFS just for system bootup
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef VFS_H
#define VFS_H

#include "c/stdint.h"

#pragma pack(1)

typedef struct struct_fs_node_t fs_node_t;

typedef struct struct_fs_op_t {
    uint32_t (*read)(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer);
    uint32_t (*write)(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer);
    fs_node_t *(*readdir)(fs_node_t *node, uint32_t index);
    fs_node_t *(*finddir)(fs_node_t *node, char *name);
    fs_node_t *(*create)(fs_node_t *node, char *name);
    fs_node_t *(*mkdir)(fs_node_t *node, char *name);
} fs_op_t;

struct struct_fs_node_t {
    char *name;
    fs_op_t *op;
    fs_node_t *pointer;
    void *dataPtr;
    uint64_t length;
    uint8_t type;
};

enum fs_node_type_t {
    FILE = 0,
    DIR = 1,
    SYMLINK = 2,
    BLOCK = 3,
    CHAR = 4,
    PIPE = 5,
    /* Mounted can coexist with above types */
    MOUNTED = 8
};

uint32_t vfs_read(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer);
uint32_t vfs_write(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer);
fs_node_t *vfs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *vfs_finddir(fs_node_t *node, char *name);
fs_node_t *vfs_create(fs_node_t *node, char *name);
fs_node_t *vfs_mkdir(fs_node_t *node, char *name);
fs_node_t *vfs_lookup(char *path);
void vfs_mount(char *path, fs_node_t *node);
void vfs_init(void);

#endif
