
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"
#include "c/string.h"
#include "bootmgr/vfs.h"

typedef struct {
    uint8_t length;
    uint8_t extLength;
    uint32_t lbaL;
    uint32_t lbaM;
    uint32_t sizeL;
    uint32_t sizeM;
    struct {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t timeZone;
    } date;
    uint8_t flags;
    uint8_t blah;
    uint8_t blah1;
    uint16_t volSeqNumL;
    uint16_t volSeqNumM;
    uint8_t nameLen;
    char fileName[1];
}__attribute__((packed)) dir_rec_t;

typedef struct {
    uint8_t typeCode;
    char identifier[5];
    uint8_t version;
    uint8_t unused;
    char sysId[32];
    char volId[32];
    uint64_t unused1;
    uint32_t volSpaceSizeL;
    uint32_t volSpaceSizeM;
    char unused2[32];
    uint16_t volSetSizeL;
    uint16_t volSetSizeM;
    uint16_t volSeqNumL;
    uint16_t volSeqNumM;
    uint16_t logicalBlockSizeL;
    uint16_t logicalBlockSizeM;
    uint32_t pathTableSizeL;
    uint32_t pathTableSizeM;
    uint32_t pathTableLocL;
    uint32_t pathTableLocOptL;
    uint32_t pathTableLocM;
    uint32_t pathTableLocOptM;
    dir_rec_t rootDir;
}__attribute__((packed)) prim_vol_desc_t;

typedef struct {
    char id[2];
    uint8_t length;
}__attribute__((packed)) sys_use_entry_t;

typedef struct {
    fs_node_t *cdrom;
    fs_node_t **cache;
    dir_rec_t dirent;
} cdfs_data_t;

#define BUFFER_SIZE 2048
#define NODE_BUFFER_SIZE 128

static uint64_t read(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer);
static fs_node_t *readdir(fs_node_t *node, uint32_t index);

static fs_op_t ops = {
    .read = read,
    .write = NULL,
    .readdir = readdir,
    .finddir = NULL,
    .create = NULL,
    .mkdir = NULL
};

static char *buffer = NULL;
static fs_node_t **nodeBuffer = NULL;

static uint64_t read(fs_node_t *node, uint64_t offset, uint64_t size, void *buffer) {
    assert(node->type != DIR);
    cdfs_data_t *data = node->dataPtr;
    if (offset > data->dirent.sizeL) {
        return 0;
    }
    if (offset + size > data->dirent.sizeL) {
        size = data->dirent.sizeL - offset;
    }
    return vfs_read(data->cdrom, data->dirent.lbaL * 2048 + offset, size, buffer);
}

static fs_node_t *readdir(fs_node_t *node, uint32_t index) {
    assert(node->type == DIR);
    cdfs_data_t *data = node->dataPtr;
    if (data->cache == NULL) {
        assert(data->dirent.sizeL <= BUFFER_SIZE);
        size_t id = 0;

        vfs_read(data->cdrom, data->dirent.lbaL * 2048, 2048, buffer);
        for (dir_rec_t *rec = (dir_rec_t *)buffer; rec->length != 0;
                rec = (dir_rec_t *)((size_t)rec + rec->length)) {
            if (*rec->fileName < 2) {
                /* 0 -> Current directory
                 * 1 -> Parent directory
                 * >=2 -> Normal file
                 */
                continue;
            }

            cdfs_data_t *newdata = malloc(sizeof(cdfs_data_t));
            memcpy(&newdata->dirent, rec, sizeof(dir_rec_t));
            newdata->cdrom = data->cdrom;
            newdata->cache = NULL;

            fs_node_t *node = malloc(sizeof(fs_node_t));
            node->name = NULL;
            node->op = &ops;
            node->pointer = NULL;
            node->dataPtr = newdata;
            node->length = newdata->dirent.sizeL;
            node->type = (rec->flags & 2) ? DIR : FILE;

            /* The spaces after file name is system use entry(SUE) */
            int lenReal = (size_t)(&rec->fileName[rec->nameLen]) - (size_t)rec;
            /* We need to pad one if the address is not 2-aligned */
            if (lenReal & 1)
                lenReal++;
            int lenRest = rec->length - lenReal;
            sys_use_entry_t *sue = (sys_use_entry_t *)((size_t)rec + lenReal);
            while (lenRest > 3) {
                lenRest -= sue->length;
                if (sue->id[0] == 'N' && sue->id[1] == 'M') {
                    node->name = strndup((void *)sue + 5, sue->length - 5);
                }
                sue = (sys_use_entry_t *)((size_t)sue + sue->length);
            }

            if (node->name == NULL) {
                node->name = strndup(rec->fileName, rec->nameLen);
            }

            nodeBuffer[id++] = node;
        }
        nodeBuffer[id++] = NULL;

        assert(id <= NODE_BUFFER_SIZE);

        data->cache = malloc(id * sizeof(fs_node_t *));
        memcpy(data->cache, nodeBuffer, id * sizeof(fs_node_t *));

        //free(buffer);
    }
    return data->cache[index];
}

fs_node_t *CDFS_create_fs(fs_node_t *cdrom) {
    if (buffer == NULL) {
        buffer = malloc(BUFFER_SIZE);
        nodeBuffer = malloc(NODE_BUFFER_SIZE * sizeof(fs_node_t *));
    }

    cdfs_data_t *data = malloc(sizeof(cdfs_data_t));
    vfs_read(cdrom, 0x8000 + offsetof(prim_vol_desc_t, rootDir), sizeof(dir_rec_t), &data->dirent);
    data->cdrom = cdrom;
    data->cache = NULL;

    fs_node_t *node = malloc(sizeof(fs_node_t));
    node->name = "cdfs";
    node->op = &ops;
    node->pointer = NULL;
    node->dataPtr = data;
    node->length = data->dirent.sizeL;
    node->type = DIR;

    return node;
}
