#include "bootmgr/vfs.h"

#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"

#define debugVar(var) printf(#var "=%d\n", (var));

int main() {
    /* Read js from disk */
    fs_node_t *node = vfs_lookup("/media/boot/saki/bootmgr/boot.js");
    char *buffer = malloc(node->length + 1);
    vfs_read(node, 0, node->length, buffer);
    buffer[node->length] = 0;

    lex_t *lex = lex_new(buffer);
    lex_next(lex);

    return 0;
}

