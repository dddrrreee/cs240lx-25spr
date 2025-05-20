#ifndef __MY_ELF_H__
#define __MY_ELF_H__

/*
     <ELF Format>

    ========================
    | ELF Header           |
    ========================
    | Program Header Table |
    ========================
    | Section 1            |
    ========================
    | Section 2            |
    ========================
    | ...                  |
    ========================
    | Section n            |
    ========================
    | Section Header Table |
    ========================
*/

#define E_NIDENT 16
#define SHT_DYNAMIC 6
#define SHT_REL 9
#define SHT_NOBITS 8
#define SHT_DYNSYM 11

#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_STRTAB 5  // .dynstr
#define DT_SYMTAB 6  // .dynsym
#define DT_JMPREL 23 // .rel.dyn

// I had to reverse-engineer these values (different from the ELF docs)
#define R_ARM_RELATIVE 23
#define R_ARM_GLOB_DAT 21
#define R_ARM_JUMP_SLOT 22
#define R_ARM_ABS32 2
#define SHN_UNDEF 0

/*
    ELF Headers
*/

// ELF Header
typedef struct {
    uint8_t     e_ident[E_NIDENT];
    uint16_t    e_type;
    uint16_t    e_machine;
    uint32_t    e_version;
    uint32_t    e_entry;
    uint32_t    e_phoff;
    uint32_t    e_shoff;
    uint32_t    e_flags;
    uint16_t    e_ehsize;
    uint16_t    e_phentsize;
    uint16_t    e_phnum;
    uint16_t    e_shentsize;
    uint16_t    e_shnum;
    uint16_t    e_shstrndx;
} elf32_header; // 52 bytes (0x34)

// Program Header
typedef struct {
    uint32_t    p_type;
    uint32_t    p_offset; 
    uint32_t    p_vaddr; 
    uint32_t    p_paddr; 
    uint32_t    p_filesz; 
    uint32_t    p_memsz; 
    uint32_t    p_flags; 
    uint32_t    p_align;
} elf32_pheader; // 32 bytes (0x20)

// Section Header
typedef struct {
    uint32_t    sh_name; 
    uint32_t    sh_type; 
    uint32_t    sh_flags; 
    uint32_t    sh_addr; 
    uint32_t    sh_offset; 
    uint32_t    sh_size; 
    uint32_t    sh_link; 
    uint32_t    sh_info; 
    uint32_t    sh_addralign; 
    uint32_t    sh_entsize;
} elf32_sheader; // 40 bytes (0x28)

// .dynamic section entry
typedef struct {
    int32_t   d_tag; // identifies the type of entry (e.g., .hash)
    union {          // based on d_tag, either a value or a pointer
        uint32_t  d_val;
        uint32_t  d_ptr;
    } d_un;
} elf32_dynamic; // 8 bytes (per entry)

// Symbol table (.dynsym, .sym) entry
typedef struct {
    uint32_t    st_name; // index into the string table
    uint32_t    st_value; 
    uint32_t    st_size; 
    uint8_t     st_info; 
    uint8_t     st_other; 
    uint16_t    st_shndx;
} elf32_sym; // 16 bytes (0x10)

// Relocation table (.rel.dyn, .rel.plt) entry
typedef struct {
    uint32_t    r_offset; 
    uint32_t    r_info; 
} elf32_rel; // 8 bytes (0x8)

// Helper struct (no need to use this! Purely for convenience)
// Just a bunch of useful addresses.
typedef struct {
    // Headers
    elf32_header   *e_header;
    elf32_pheader  *e_pheaders;
    elf32_sheader  *e_sheaders;

    // Essential sections for dynamic linking
    elf32_dynamic *e_dynamics;
    uint32_t       n_dynamics; // number of entries in .dynamic section
    uint32_t      *e_hash;
    elf32_sym     *e_dynsym;
    char          *e_dynstr;
    uint32_t      *e_pltgot;
    elf32_rel     *e_reldyn;
} my_elf32;


// Load the ELF file from the SD card to memory, starting at address `base`
void load_elf(char *filename, char *base);

// Verify the ELF header. We'll do only a few checks as an exercise.
// Refer to 1-3 of ELF.pdf for the ELF header format
void verify_elf(elf32_header *e_header);

// Zero-initialize the .bss section
// In order to do this, we need to:
//   - Find out where the section header table starts from the ELF header
//   - Iterate through the section header table entries until we find the .bss section
//      - Thankfully, the .bss section is the only section that has sh_type == SHT_NOBITS
//        for our case. So we can rely on this for now.
//   - Zero-initialize the .bss section
// Refer to 1-4, 1-9, 1-10, and 1-13
void bss_zero_init(elf32_header *e_header);

// Jump to the ELF file's entry point.
void jump_to_elf_entry(elf32_header *e_header);

#endif
