#include "types.h"


// Executable and Linkable Format(ELF) ———— 可执行与可链接格式

// identify whether the file is a valid ELF file
#define ELF_MAGIC 0x464C457FU   // "\x7FELF" in little endian

// ELF file header
struct ELFhdr {
    uint magic;                 // must equal ELF_MAGIC
    uchar elf[12];              // other information of the ELF file
    ushort type;                // type of file
    ushort machine;             // machine architecture, x86/ARM
    uint version;               // version of ELF file
    uint entry;                 // entry point address of program
    uint phoff;                 // offset of program Header(locate the position of program Header in file)
    uint shoff;                 // offset of Section Header(locate the position of Section Header in file)
    uint flags;                 // flag bits
    ushort ehsize;              // size of ELF file header
    ushort phentsize;           // size of each program Header in program Header Table
    ushort phnum;               // number of program Headers in program Header Table
    ushort shentsize;           // size of each Section Header in Section Header Table
    ushort shnum;               // number of Section Headers in Section Header Table
    ushort shstrndx;            // index of the Section Header String Table
  };

  // program header
  struct Proghdr {
    uint type;                  // type of program segment
    uint off;                   // offset of segment in ELF file
    uint vaddr;                 // virtual address of segment in memory
    uint paddr;                 // physical address of segment in memory
    uint filesz;                // size of segment in ELF file(bytes)
    uint memsz;                 // size of segment in memory(bytes)
    uint flags;                 // flag bit 
    uint align;                 // alignment method of segment
  };

// type of program header
#define ELF_PROG_LOAD           1

// flags of program header
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4