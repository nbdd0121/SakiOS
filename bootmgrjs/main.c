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

    while (1) {
        token_t *ret = lex_next(lex);
        if (ret->type < ASSIGN_FLAG) {
            printf("(%c)", ret->type);
        } else if (ret->type < DOUBLE_FLAG) {
            printf("(%c=)", ret->type & ~ ASSIGN_FLAG);
        } else if (ret->type < OTHER_FLAG) {
            printf("(%c=)", ret->type & ~ DOUBLE_FLAG);
        } else {
            switch (ret->type) {
                case FULL_EQ: {
                    printf("(===)");
                    break;
                }
                case FULL_INEQ: {
                    printf("(!==)");
                    break;
                }
                case SHL: {
                    printf("(<<)");
                    break;
                }
                case SHR: {
                    printf("(>>)");
                    break;
                }
                case USHR: {
                    printf("(>>>)");
                    break;
                }
                case SHL_ASSIGN: {
                    printf("(<<=)");
                    break;
                }
                case SHR_ASSIGN: {
                    printf("(>>=)");
                    break;
                }
                case USHR_ASSIGN: {
                    printf("(>>>=)");
                    break;
                }
                case STR: {
                    utf8_string_t str = unicode_toUtf8(ret->stringValue);
                    printf("(STR, %.*s)", str.len, str.str);
                    break;
                }
                case ID: {
                    utf8_string_t str = unicode_toUtf8(ret->stringValue);
                    printf("(ID, %.*s)", str.len, str.str);
                    break;
                }
                case NUM: {
                    printf("(NUM, %d)", (size_t)ret->numberValue);
                    break;
                }
                case LINE: {
                    printf("(LINE)");
                    break;
                }
                default:
                    assert(0);
            }
        }
    }

    return 0;
}

