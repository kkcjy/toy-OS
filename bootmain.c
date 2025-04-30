/*  ···加载流程···
    与 boot_first_cpu.S 共同构成 boot 程序
    boot_first_cpu.S 调用 bootmain()
    bootmain() 从磁盘首个扇区开始加载一个 ELF 格式的内核镜像，然后跳转到内核入口
*/
#pragma once
#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"


// size of sector
#define SECTSIZE  512


// wait for disk ready
void waitdisk(void) {
    /*
    7'th bit (busy)     
    6'th bit (Drive Ready)
    disk is not busy && ready -> break
    */
    while((inb(0x1F7) & 0xC0) != 0x40);
}

// read a single sector at offset into dst
void readsect(void *dst, uint offset) {
    waitdisk();
    outb(0x1F2, 1);                         // counter of sector
    outb(0x1F3, offset);                    // read offset[7..0]
    outb(0x1F4, offset >> 8);               // read offset[15..8]
    outb(0x1F5, offset >> 16);              // read offset[23..16]
    outb(0x1F6, (offset >> 24) | 0xE0);     // read offset[31..24] and set parameter
    outb(0x1F7, 0x20);                      // cmd 0x20 - read sectors

    waitdisk();
    // read SECTSIZE/4 times 32-bit data(a sector) to dst from 0x1F0
    insl(0x1F0, dst, SECTSIZE/4);
}

// read count bytes at offset from kernel into physical address pa
void readseg(uchar* pa, uint count, uint offset) {
    uchar *epa = pa + count;

    /*
    磁盘控制器无法直接从扇区中间开始读取数据，只能从扇区的边界开始读取
    需要将物理地址 pa 向下舍入到扇区边界
    */
    pa -= offset % SECTSIZE;

    // translate offset from bytes to sectors
    // kernel files start at sector sector 1
    offset = (offset / SECTSIZE) + 1;
  
    for(; pa < epa; pa += SECTSIZE){
        readsect(pa, offset);
        offset++;
    }
}

void bootmain(void) {
    /*
    define an entry to store address of program entry point specified in ELF file
    program will jump to kernel's entry point through this pointer and start executing kernel code
    */
    void (*entry)(void);

    struct ELFhdr *elfhdr;
    elfhdr = (struct ELFhdr*)0x10000;

    struct Proghdr *proghdr, *eproghdr;

    uchar* pa;

    // read 1st page off disk from 0 to elfhdr
    readseg((uchar*)elfhdr, 4096, 0);

    // if an ELF file, not -> let bootasm.S handle error
    if(elfhdr->magic != ELF_MAGIC)
        return;  

    // proghdr = elfhdr + elfhdr->phoff
    proghdr = (struct Proghdr*)((uchar*)elfhdr + elfhdr->phoff);
    // eproghdr = proghdr + elfhdr->phnum * sizeof(Proghdr)
    eproghdr = proghdr + elfhdr->phnum;

    // traverse program header table and load each segment into memory
    for(; proghdr < eproghdr; proghdr++){
        pa = (uchar*)proghdr->paddr;
        readseg(pa, proghdr->filesz, proghdr->off);
        /*
        in addition to storing data from file, program segment may
        also need require additional space in memory to store some 
        uninitialized data which needed to be initialized to 0
        */
        if(proghdr->memsz > proghdr->filesz)
        // 8-bit of 0 * (proghdr->memsz - proghdr->filesz) -> (pa + proghdr->filesz)
            stosb(pa + proghdr->filesz, 0, proghdr->memsz - proghdr->filesz);
    }

    // call entry point
    entry = (void(*)(void))(elfhdr->entry);
    entry();
}