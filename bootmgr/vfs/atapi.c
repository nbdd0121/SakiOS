#include "c/stdint.h"
#include "c/stdbool.h"
#include "c/assert.h"
#include "c/string.h"
#include "c/stdlib.h"
#include "util/alignment.h"
#include "util/endian.h"
#include "asm/asm.h"
#include "bootmgr/vfs.h"

enum {
    ATA_PRIMARY = 0x1F0,
    ATA_SECONDARY = 0x170,
    ATA_MASTER = 0,
    ATA_SLAVE = 1 << 4,

    ATA_DATA = 0,
    ATA_ERROR = 1,
    ATA_FEATURE = 1,
    ATA_SECTOR_COUNT = 2,
    ATA_LBA_LOW = 3,
    ATA_LBA_MID = 4,
    ATA_LBA_HIGH = 5,
    ATA_DEV = 6,
    ATA_STATUS = 7,
    ATA_COMMAND = 7,
    ATA_ALT_STATUS = 0x206,
    ATA_DEV_CTRL = 0x206,
};

typedef struct {
    int bus;
    int dev;
} atapi_data_t;

static uint64_t read(fs_node_t *node, uint64_t _offset, uint64_t _size, void *buffer);

static int dev[2] = { -1, -1};

static atapi_data_t device = {
    .bus = ATA_SECONDARY,
    .dev = ATA_MASTER
};

static fs_op_t op = {
    .read = read,
    .write = NULL,
    .readdir = NULL,
    .finddir = NULL,
    .create = NULL,
    .mkdir = NULL
};
static fs_node_t node = {
    .name = "cdrom",
    .op = &op,
    .pointer = NULL,
    .dataPtr = &device,
    //.length will be set by code
    .type = BLOCK
};

static char *buf = NULL;

static void ATA_delay(atapi_data_t *data, int delay) {
    for (; delay; delay--)
        readPort8(data->bus + ATA_ALT_STATUS);
}

static void ATA_select(atapi_data_t *data) {
    if (dev[data->bus == ATA_PRIMARY ? 0 : 1] == data->dev) {
        return;
    }
    dev[data->bus == ATA_PRIMARY ? 0 : 1] = data->dev;
    writePort8(data->bus + ATA_DEV, data->dev);
    ATA_delay(data, 4);
}

static int ATA_waitDevice(atapi_data_t *data) {
    uint8_t status;
    while (true) {
        status = readPort8(data->bus + ATA_STATUS);
        if ((status & (1 << 7)) == 0) {
            break;
        } else if ((status & 1) != 0) {
            return 0;
        }
    }
    while (true) {
        status = readPort8(data->bus + ATA_STATUS);
        if ((status & (1 << 3)) != 0) {
            return 1;
        } else if ((status & 1) != 0) {
            return 0;
        }
    }
    // Should never reach
    return 0;
}

static bool ATAPI_identify(atapi_data_t *data) {
    ATA_select(data);
    writePort8(data->bus + ATA_COMMAND, 0xA1);
    if (readPort8(data->bus + ATA_STATUS) == 0) {
        return false;
    }
    if (!ATA_waitDevice(data)) {
        return false;
    }
    repReadPort16(data->bus + ATA_DATA, buf, 512);

    // Maybe we can do something here.
    return true;
}

static int ATAPI_packet(atapi_data_t *data, int byteCount, char *packet, int size) {
    ATA_select(data);
    writePort8(data->bus + ATA_FEATURE, 0);
    writePort8(data->bus + ATA_DEV_CTRL, 2);
    writePort8(data->bus + ATA_LBA_MID, byteCount & 0xFF);
    writePort8(data->bus + ATA_LBA_HIGH, byteCount >> 8);
    writePort8(data->bus + ATA_COMMAND, 0xA0);
    if (!ATA_waitDevice(data)) {
        // printf("[ERROR] [ATAPI] Error happened when sent PACKET command");
        return -1;
    }
    repWritePort16(data->bus + ATA_DATA, packet, size);
    if (!ATA_waitDevice(data)) {
        // printf("[ERROR] [ATAPI] Error happended when packet sent (code %x)", readPort8(data->bus + ATA_ERROR));
        return -1;
    }
    return readPort8(data->bus + ATA_LBA_MID) | (readPort8(data->bus + ATA_LBA_HIGH) << 8);
}

static uint32_t ATAPI_readCapacity(atapi_data_t *data) {
    char packet[10] = {0x25};

    int count = ATAPI_packet(data, 8, packet, 12);
    if (count == -1)return 0;

    uint32_t buffer[2];
    repReadPort16(data->bus + ATA_DATA, buffer, count);

    buffer[0] = endian32(buffer[0]);
    buffer[1] = endian32(buffer[1]);

    uint32_t size = (buffer[0] + 1) * buffer[1];
    printf("[INFO] [ATAPI] Capacity: %dKB, %dKiB\n", size / 1000, size / 1024);
    return size;
}

static bool ATAPI_readSector(atapi_data_t *data, char *buffer, uint32_t lba) {
    char packet[12] = {0xA8, [9] = 1};
    packet[2] = (lba >> 24) & 0xFF;
    packet[3] = (lba >> 16) & 0xFF;
    packet[4] = (lba >> 8) & 0xFF;
    packet[5] = lba & 0xFF;
    int count = ATAPI_packet(data, 2048, packet, 12);
    assert(count == 2048);
    repReadPort16(data->bus + ATA_DATA, buffer, count);
    return true;
}

static uint64_t read(fs_node_t *node, uint64_t _offset, uint64_t _size, void *buffer) {
    uint32_t offset = (uint32_t)_offset;
    uint32_t size = (uint32_t)_size;
    atapi_data_t *data = node->dataPtr;
    if (alignTo(offset, 2048) != offset) {
        uint32_t alignU = alignTo(offset, 2048);
        uint32_t alignD = alignDown(offset, 2048);
        uint32_t nsize = alignU - offset;
        ATAPI_readSector(data, buf, alignD / 2048);
        if (nsize > size) {
            memcpy(buffer, buf + offset - alignD, size);
            return size;
        } else {
            memcpy(buffer, buf + offset - alignD, nsize);
            read(node, alignU, size - nsize, buffer + nsize);
        }
        return size;
    }
    if (size == 2048) {
        ATAPI_readSector(data, buffer, offset / 2048);
    } else if (size > 2048) {
        ATAPI_readSector(data, buffer, offset / 2048);
        read(node, offset + 2048, size - 2048, buffer + 2048);
    } else {
        ATAPI_readSector(data, buf, offset / 2048);
        memcpy(buffer, buf, size);
    }
    return size;
}

fs_node_t *ATAPI_init(void) {
    buf = malloc(2048);
    printf("[ATAPI BUFFER %p]", buf);

    if (ATAPI_identify(&device)) {
        printf("[INFO] [ATAPI]: Secondary Master detected\n");
        node.length = ATAPI_readCapacity(&device);
        return &node;
    }

    device.dev = ATA_SLAVE;
    if (ATAPI_identify(&device)) {
        printf("[INFO] [ATAPI]: Secondary Slave detected\n");
        node.length = ATAPI_readCapacity(&device);
        return &node;
    }

    device.bus = ATA_PRIMARY;
    if (ATAPI_identify(&device)) {
        printf("[INFO] [ATAPI] Primary Slave detected\n");
        node.length = ATAPI_readCapacity(&device);
        return &node;
    }

    device.dev = ATA_MASTER;
    if (ATAPI_identify(&device)) {
        printf("[INFO] [ATAPI] Primary Master detected\n");
        node.length = ATAPI_readCapacity(&device);
        return &node;
    }

    assert(0);
    return 0;
}
