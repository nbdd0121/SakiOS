#ifndef ELF_ELF32_H
#define ELF_ELF32_H

#include "c/stdint.h"

enum {
    EI_MAG0 = 0 ,
    EI_MAG1 = 1 ,
    EI_MAG2 = 2 ,
    EI_MAG3 = 3 ,
    EI_CLASS = 4 ,
    EI_DATA = 5 ,
    EI_VERSION = 6 ,
    EI_OSABI = 7 ,
    EI_ABIVERSION = 8 ,
    EI_PAD = 9 ,
    EI_NIDENT = 16
};

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word, Elf32_Off, Elf32_Addr;

typedef struct {
    uint8_t e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}__attribute__((packed)) Elf32_Ehdr;

typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
}__attribute__((packed)) Elf32_Shdr;

enum {
    SHT_SYMTAB = 2,
    SHT_STRTAB = 3,
    SHT_NOBITS = 8,
    SHT_REL = 9
};

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
}__attribute__((packed)) Elf32_Rel;

typedef struct {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    uint8_t st_info;
    uint8_t st_other;
    Elf32_Half st_shndx;
}__attribute__((packed)) Elf32_Sym;

enum {
    STT_SECTION = 3
};

enum {
    STB_LOCAL       = 0,
    STB_GLOBAL      = 1,
    STB_WEAK        = 2
};

enum {
    SHN_UNDEF = 0,
    SHN_ABS = 0xfff1,
    SHN_COMMON = 0xfff2
};

enum {
    SHF_ALLOC = 0x2

};

#define ELF32_R_SYM(info)             ((info)>>8)
#define ELF32_R_TYPE(info)            ((unsigned char)(info))

#define ELF32_ST_BIND(info)          ((info) >> 4)
#define ELF32_ST_TYPE(info)          ((info) & 0xf)

#define ELF32_SH_GET(header, index) \
    ((Elf32_Shdr*)((void*)(header)+(header)->e_shoff+(header)->e_shentsize*(index)))

#define ELF32_SH_LINK(header, section) ELF32_SH_GET(header, (section)->sh_link)
#define ELF32_SH_CONTENT(header, section) ((void*)(header)+(section)->sh_offset)

enum {
    R_386_32 = 1,
    R_386_PC32 = 2
};

#endif
