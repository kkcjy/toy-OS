#include "types.h"
// Eflags register —— 处理器标志寄存器
// IF —— 中断允许标志位 \ EFLAGS register 第 9 位
#define FL_IF           0x00000200          // Interrupt Enable 

// various segment selectors
#define SEG_KCODE       1                   // kernel code
#define SEG_KDATA       2                   // kernel data+stack
#define SEG_UCODE       3                   // user code
#define SEG_UDATA       4                   // user data+stack
// #define SEG_TSS         5                   // this process's task state
#define NSEGS           6                   // cpu->gdt[NSEGS] holds above segments

#ifndef __ASSEMBLER__
// Segment Descriptor
struct segdesc {
    uint lim_15_0 : 16;                     // low bits of segment limit
    uint base_15_0 : 16;                    // low bits of segment base address
    uint base_23_16 : 8;                    // middle bits of segment base address
    uint type : 4;                          // segment type
    uint s : 1;                             // 0 -> system segment, 1 -> application segment
    uint dpl : 2;                           // descriptor Privilege Level
    uint p : 1;                             // present
    uint lim_19_16 : 4;                     // high bits of segment limit
    uint avl : 1;                           // unused (available for software use)
    uint rsv1 : 1;                          // reserved (available for hardware use)
    uint db : 1;                            // 0 -> 16-bit segment, 1 -> 32-bit segment
    uint g : 1;                             // for segment limit: 0 -> units of 1B, 1 -> units of 4KB
    uint base_31_24 : 8;                    // High bits of segment base address
};
#endif

#define DPL_USER        0x3                 // User DPL

// page directory and page table constants
#define PGSIZE          4096                // bytes mapped by a page
