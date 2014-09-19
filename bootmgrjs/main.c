#include "bootmgr/vfs.h"

#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/type.h"

#include "c/math.h"

#define debugVar(var) printf(#var "=%d\n", (var));

int main() {
    /* Read js from disk */
    fs_node_t *node = vfs_lookup("/media/boot/saki/bootmgr/boot.js");
    char *buffer = malloc(node->length + 1);
    vfs_read(node, 0, node->length, buffer);
    buffer[node->length] = 0;

    js_init();

    lex_t *lex = lex_new(buffer);
    grammar_t *gmr = grammar_new(lex);

    //grammar_program(gmr);
    js_context_t context = {
        .thisBinding = js_createGlobal()
    };

    js_data_t *se = grammar_exprStmt(gmr);
    js_data_t *ret = js_getValue(js_evalNode(&context, se));
    assert(0);

    js_string_t *str = js_toString(ret);
    unicode_putUtf16(str->value);

    //js_toString(js_new_number(12345));

    return 0;
}

