#include "c/stdio.h"

int main() {
    printf("This is js");

    EXPORT(vfs_read);
    EXPORT(vfs_write);
    EXPORT(vfs_readdir);
    EXPORT(vfs_finddir);
    EXPORT(vfs_create);
    EXPORT(vfs_mkdir);
    EXPORT(vfs_lookup);
    EXPORT(vfs_mount);
    EXPORT(vfs_mount_fs);
    EXPORT(vfs_init);

    return 0;
}
