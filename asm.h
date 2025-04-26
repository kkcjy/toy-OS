// assembler macros to create x86 segments

/*
define an empty segment descriptor to occupy a position, 
preventing the use of uninitialized segment descriptors
segment descriptor used to describe the attributes and ranges of memory segments
*/
#define SEG_NULLASM                                            \
       .word 0, 0;                                             \
       .byte 0, 0, 0, 0

/*
define a complete segment descriptor
    type - type of segment(executable/non-executable)
    base - starting address of segment
    lim  - limit size of segment

    .word set:
        lower 16 bits of segment limit 
        lower 16 bits of segment base address

    .byte set:
        middle 8 bits of segment base address
        segment type
        upper 4 bits of segment limit
        upper 8 bits of segment base address
*/
#define SEG_ASM(type,base,lim)                                 \
       .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
       .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
               (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define STA_X     0x8       // executable segment

#define STA_W     0x2       // writeable (non-executable segments)

#define STA_R     0x2       // readable (executable segments)