
#include "c/stdio.h"
#include "c/string.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "data-struct/hashmap.h"

#include "elf/elf32.h"

hashmap_t *symbolMap = NULL;

#define HASH_POOL_SIZE 31

void add_symbol(char *name, void *addr) {
    if (symbolMap == NULL) {
        symbolMap = hashmap_new_string(HASH_POOL_SIZE);
    }
    hashmap_put(symbolMap, name, addr);
}

static uint32_t resolve_external_symbol(char *name) {
    if (symbolMap == NULL) {
        return 0;
    }
    void *ent = hashmap_get(symbolMap, name);
    return (uint32_t)ent;
}

static uint32_t resolve_symbol(Elf32_Ehdr *header, char *strtab, Elf32_Sym *symbol) {
    switch (symbol->st_shndx) {
        case SHN_ABS: {
            return symbol->st_value;
        }
        case SHN_UNDEF: {
            char *name = strtab + symbol->st_name;
            uint32_t ret = resolve_external_symbol(name);
            if (ret) {
                /* Cache the result */
                symbol->st_shndx = SHN_ABS;
                symbol->st_value = ret;
                return ret;
            }
            if (!(ELF32_ST_BIND(symbol->st_info) & STB_WEAK)) {
                printf("[ERROR] [ELF] Failed to resolve %s\n", name);
                assert(0);
            }
            /* Cache the result */
            symbol->st_shndx = SHN_ABS;
            symbol->st_value = 0;
            return 0;
        }
        case SHN_COMMON: {
            void *ret = malloc(symbol->st_size);
            memset(ret, 0, symbol->st_size);
            symbol->st_shndx = SHN_ABS;
            symbol->st_value = (uint32_t)ret;
            return (uint32_t)ret;
        }
        default: {
            uint32_t ret = (uint32_t)
                           ELF32_SH_CONTENT(header, ELF32_SH_GET(header, symbol->st_shndx)) +
                           symbol->st_value;
            /* Cache the result */
            symbol->st_shndx = SHN_ABS;
            symbol->st_value = ret;
            return ret;
        }
    }
}

void link_elf32(void *content) {
    Elf32_Ehdr *header = content;
    // char *shstrtab = ELF32_SH_GET(header, header->e_shstrndx)->sh_offset + content;
    for (int i = 0; i < header->e_shnum; i++) {
        Elf32_Shdr *section = ELF32_SH_GET(header, i);
        switch (section->sh_type) {
            case SHT_REL: {
                // printf("[Section rel %s, ", shstrtab + section->sh_name);
                Elf32_Shdr *symbolSection = ELF32_SH_LINK(header, section);
                Elf32_Shdr *targetSection = ELF32_SH_GET(header, section->sh_info);
                // printf("symbol %s, ", shstrtab + symbolSection->sh_name);
                // printf("target %s]\n", shstrtab + targetSection->sh_name);

                char *symStrSect = ELF32_SH_CONTENT(header,
                                                    ELF32_SH_LINK(header, symbolSection));
                Elf32_Sym *symbolTable = ELF32_SH_CONTENT(header, symbolSection);

                int size = section->sh_size / sizeof(Elf32_Rel);
                Elf32_Rel *rel = content + section->sh_offset;
                for (int i = 0; i < size; i++) {
                    Elf32_Sym *assoc = &symbolTable[ELF32_R_SYM(rel[i].r_info)];
                    uint32_t *ref = ELF32_SH_CONTENT(header, targetSection) + rel[i].r_offset;
                    switch (ELF32_R_TYPE(rel[i].r_info)) {
                        case R_386_32: {
                            uint32_t S = (uint32_t)resolve_symbol(header, symStrSect, assoc);
                            uint32_t A = *ref;
                            // printf("S[%x] + A[%x] -> %p", S, A, ref);
                            *ref = S + A;
                            break;
                        }
                        case R_386_PC32: {
                            uint32_t S = (uint32_t)resolve_symbol(header, symStrSect, assoc);
                            uint32_t A = *ref;
                            uint32_t P = (uint32_t)ref;
                            // printf("S[%x] + A[%x] - P[%x] -> %p", S, A, P, ref);
                            *ref = S + A - P;
                            break;
                        }
                        default: {
                            assert(0);
                        }
                    }

                    // printf("[sym %s]\n", symStrSect + assoc->st_name);
                }
            }
        }
    }
}

int exec_elf32(void *content) {
    Elf32_Ehdr *header = content;
    //char *shstrtab = ELF32_SH_GET(header, header->e_shstrndx)->sh_offset + content;

    void *entrance = NULL;

    for (int i = 0; i < header->e_shnum; i++) {
        Elf32_Shdr *section = ELF32_SH_GET(header, i);
        switch (section->sh_type) {
            case SHT_SYMTAB: {
                char *symStrSect = ELF32_SH_CONTENT(header,
                                                    ELF32_SH_LINK(header, section));
                Elf32_Sym *symbol = ELF32_SH_CONTENT(header, section);

                int size = section->sh_size / sizeof(Elf32_Sym);
                for (int i = 0; i < size; i++) {
                    if (strcmp(symbol[i].st_name + symStrSect, "main") == 0) {
                        entrance = ELF32_SH_CONTENT(header,
                                                    ELF32_SH_GET(header, symbol[i].st_shndx))
                                   + symbol[i].st_value;
                    }
                }
            }
        }
    }

    assert(entrance);

    return ((int(*)(void))entrance)();
}