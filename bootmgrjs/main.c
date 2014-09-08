#include "c/stdio.h"
#include "c/stdint.h"

#include "unicode/category.h"

#define debugVar(var) printf(#var "=%d\n", (var));

int main() {
    printf("This is js");

    debugVar(unicode_getType(L'Ā'));
    debugVar(UPPERCASE_LETTER);

    return 0;
}

