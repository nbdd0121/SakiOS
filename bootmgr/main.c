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
#include "c/math.h"

#include "mem-alloc/pageman.h"
#include "c-stdlib/malloc.h"
#include "asm/asm.h"

#include "data-struct/hashmap.h"

#include "bootmgr/vfs.h"

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
#define AVAIL_MEM_START (32*0x100000)

void add_symbol(char *, void *);
void link_elf32(void *);
int exec_elf32(void *);

/**
 * C code entrance.
 * Notice that void main(void) is not violence of C standard,
 * because this is not in a hosted environment.
 */
void main(void) {
    /* Clear the screen */
    putchar('\f');

    uint32_t maxAddr = 0;
    /* We assume the memory maps are arranged in ascending order */
    for (int i = memMapEntryLen - 1; i >= 0; i--) {
        memmap_entry_t *entry = &memMapPtr[i];
        if (entry->type == 1) {
            uint64_t limit = entry->base + entry->limit;
            if (limit > 0xFFFFFFFF) {
                maxAddr = 0xFFFFFFFF;
            } else {
                maxAddr = (uint32_t)limit;
            }
            break;
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
                size_t base = entry->base;
                size_t limit = entry->limit;
                if (base < AVAIL_MEM_START) {
                    if (base + limit <= AVAIL_MEM_START) {
                        continue;
                    } else {
                        limit -= AVAIL_MEM_START - base;
                        base = AVAIL_MEM_START;
                    }
                }
                if (man) {
                    pageman_freeBlock(man, (void *)(size_t)base, limit);
                } else {
                    man = pageman_create(NULL, maxAddr / PAGE_SIZE, (void *)(size_t)base, limit / PAGE_SIZE);
                    break;
                }
            }
        }
    }

    assert(man);
    init_allocator(man);

    /* Duplicate the memory map, move it to a safer location */
    memmap_entry_t *map = malloc(sizeof(memmap_entry_t) * memMapEntryLen);
    memcpy(map, memMapPtr, sizeof(memmap_entry_t)*memMapEntryLen);
    memMapPtr = map;

    /* Print all memory entries */
    for (int i = 0; i < memMapEntryLen; i++) {
        memmap_entry_t *entry = &memMapPtr[i];
        if (entry->base <= 0xFFFFFFFF) {
            if (entry->type > 5 || entry->type == 0) {
                entry->type = 2;
            }
            printf("[INFO] [MEM]: %08X %08X %s\n", (size_t)entry->base, (size_t)entry->limit,
                   typeName[entry->type - 1]);
        }
    }

    /* Create VFS and mount necessary file systems */
    vfs_init();

    vfs_mount("/", ramfs_create_fs());
    vfs_mount("/dev/cdrom", ATAPI_init());
    vfs_mount_fs("/media/cdrom/", "/dev/cdrom", CDFS_create_fs);
    vfs_mount("/media/boot/", vfs_lookup("/media/cdrom/"));

    /* Display currently available memories */
    size_t spare = pageman_spare(man);
    printf("[INFO] [MEM]: Available Memory: %d GiB, %d MiB, %d KiB\n",
           spare / 1024 / 1024 / 1024,
           spare / 1024 / 1024 % 1024,
           spare / 1024 % 1024);


    /* Load the sakiload */
    fs_node_t *exec = vfs_lookup("/media/cdrom/saki/bootmgr/js.ske");
    void *content = malloc(exec->length);
    vfs_read(exec, 0, exec->length, content);

    /* Here we manually add symbols instead of import a symbol file from fs,
     * in order to, first, enable the gc-sections, and second, to control the
     * symbols visible to applications */
#define EXPORT(n) add_symbol(#n, n);
    /* stdlib */
    EXPORT(printf);

    EXPORT(strlen);
    EXPORT(memcmp);
    EXPORT(memset);
    EXPORT(memcpy);

    EXPORT(malloc);
    EXPORT(free);
    EXPORT(realloc);

    EXPORT(isnan);
    EXPORT(isinf);
    EXPORT(fabs);
    EXPORT(log10);
    EXPORT(pow);
    EXPORT(floor);
    EXPORT(fmod);

    /* VFS */
    EXPORT(vfs_read);
    EXPORT(vfs_write);
    EXPORT(vfs_readdir);
    EXPORT(vfs_finddir);
    EXPORT(vfs_create);
    EXPORT(vfs_mkdir);
    EXPORT(vfs_lookup);
    EXPORT(vfs_mount);
    EXPORT(vfs_mount_fs);

    /* Hashmap */
    EXPORT(hashmap_new);
    EXPORT(hashmap_put);
    EXPORT(hashmap_get);
    EXPORT(hashmap_remove);
    EXPORT(hashmap_iterator);
    EXPORT(hashmap_next);

    /* Symbol Table */
    EXPORT(add_symbol);
    EXPORT(link_elf32);
    EXPORT(exec_elf32);

    link_elf32(content);
    exec_elf32(content);
}



