#include "bootmgr/vfs.h"

#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/node.h"

#define debugVar(var) printf(#var "=%d\n", (var));

int main() {
    /* Read js from disk */
    fs_node_t *node = vfs_lookup("/media/boot/saki/bootmgr/boot.js");
    char *buffer = malloc(node->length + 1);
    vfs_read(node, 0, node->length, buffer);
    buffer[node->length] = 0;

    lex_t *lex = lex_new(buffer);
    grammar_t *gmr = grammar_new(lex);

    //grammar_program(gmr);

    node_t *se = grammar_program(gmr);
    while (se) {
        examine_node(se);
        printf("\n");
        se = se->next;
    }

    return 0;
}

