/**
 * C code entrance of boot manager.
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "mem-alloc/pageman.h"
#include "c-stdlib/malloc.h"
#include "asm/asm.h"

#include "vfs/vfs.h"

#pragma pack(1)

typedef struct {
    uint64_t base;
    uint64_t limit;
    uint32_t type;
} memmap_entry_t;

extern uint8_t memMapEntryLen;
extern memmap_entry_t *memMapPtr;

static const char *typeName[] = {
    [0] = "Memory",
    [1] = "Reversed",
    [2] = "ACPI",
    [3] = "NVS",
    [4] = "Disabled"
};

#define PAGE_SIZE 4096

/**
 * C code entrance.
 * Notice that void main(void) is not violence of C standard,
 * because this is not in a hosted environment.
 */
void main(void) {
    /* Clear the screen */
    putchar('\f');

    uint32_t maxAddr = 0;
    {
        memmap_entry_t *entry = &memMapPtr[memMapEntryLen - 1];
        uint64_t limit = entry->base + entry->limit;
        if (limit > 0xFFFFFFFF) {
            maxAddr = 0xFFFFFFFF;
        } else {
            maxAddr = (uint32_t)limit;
        }
    }

    pageman_t *man = NULL;

    /* In the following algorithm, we will ignore memory over 0xFFFFFFFF,
     * since currently we are in 32 bit mode without paging, therefore we
     * have no way to make use of these memory */
    for (int i = 0; i < memMapEntryLen; i++) {
        memmap_entry_t *entry = &memMapPtr[i];
        if (entry->type == 1) {
            if (entry->base < 0x100000) {
                /* We assume no memory block will cross 0x100000,
                 * because normally 0x9FC00-0x100000 is BIOS reserved
                 * memory area. */
                continue;
            } else if (entry->base <= 0xFFFFFFFF) {
                /* We assume no memory block will cross 0xFFFFFFFF,
                 * because normally APIC memory area will locate
                 * between 0xFFFC000 - 0xFFFFFFFF. */
                if (man) {
                    pageman_freeBlock(man, (void *)(size_t)entry->base, entry->limit);
                } else {
                    man = pageman_create(NULL, maxAddr / PAGE_SIZE, (void *)(size_t)entry->base, entry->limit / PAGE_SIZE);
                    break;
                }
            }
        }
    }

    assert(man);
    init_allocator(man);

    memmap_entry_t *map = malloc(sizeof(memmap_entry_t) * memMapEntryLen);
    memcpy(map, memMapPtr, sizeof(memmap_entry_t)*memMapEntryLen);
    memMapPtr = map;

    int i;
    for (i = 0; i < memMapEntryLen; i++) {
        memmap_entry_t *entry = &memMapPtr[i];
        if (entry->base <= 0xFFFFFFFF) {
            printf("%08X %08X %s\n", (size_t)entry->base, (size_t)entry->limit,
                   typeName[entry->type - 1]);
        }
    }

    uint32_t spare = pageman_spare(man);
    printf("%d GiB, %d MiB, %d KiB\n", spare / 1024 / 1024 / 1024, spare / 1024 / 1024 % 1024, spare / 1024 % 1024);

    vfs_init();
    vfs_mount("/", ramfs_create_fs());
    vfs_mount("/dev/cdrom", ATAPI_init());
    vfs_mount("/media/cdrom/", CDFS_create_fs(vfs_lookup("/dev/cdrom")));

    fs_node_t *root = vfs_lookup("/media/cdrom/");
    for (int i = 0;; i++) {
        fs_node_t *node = vfs_readdir(root, i);
        if (node == NULL) {
            break;
        }
        printf("[%s]", node->name);
    }

}


