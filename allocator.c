#include "spinlock.h"
#include "mmu.h"
#include "memlayout.h"


// physical memory allocator
// allocate memory for user processes, kernel stacks, page table pages, pipe buffers
// allocate 4096-byte pages


struct Run {
    struct Run *next;
};

struct Kmem {
    struct Spinlock lock;
    int use_lock;           // whether using lock
    struct Run *freelist;
} kmem;

// first address after kernel loaded from ELF file
// defined by the kernel linker script in kernel.ld
extern char end[]; 

// allocate a page of physical memory
// returns a pointer that the kernel can use
// returns 0 if the memory cannot be allocated
char* kalloc(void) {
    struct Run *run;
    if(kmem.use_lock)
        acquire(&kmem.lock);
    // allocate page
    run = kmem.freelist;
    // allocate successfully, update freelist
    if(run)
        kmem.freelist = run->next;
    if(kmem.use_lock)
        release(&kmem.lock);
    return (char*)run;
}

// free the page of physical memory pointed at by v
void kfree(char *v) {
    struct Run *r;

    // avoid illegal address
    /*
    1. v 并非页对齐
    2. v 小于内核加载后起始地址
    3. v 转换物理地址后大于物理内存上限
    */
    if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
        panic("kfree");

    // fill with junk(1) to catch dangling refs
    // (incorrect access to freed memory) -> (get junk data)
    memset(v, 1, PGSIZE);

    if(kmem.use_lock)
        acquire(&kmem.lock);
    r = (struct Run*)v;
    // fallback freelist
    r->next = kmem.freelist;
    kmem.freelist = r;
    if(kmem.use_lock)
        release(&kmem.lock);
}

// free pages of physical memory from vstart to vend
void freerange(void *vstart, void *vend) {
    char *p;
    // move from vstart to upper page boundary
    p = (char*)PGROUNDUP((uint)vstart);
    // avoid deleting pages that are still in use
    for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
        kfree(p);
}

/*
initialization happens in two phases
1. main() calls kinit1() 
using entrypgdir to place just pages mapped by entrypgdir on free list
2. main() calls kinit2() 
after installing a full page table that maps rest of the physical pages on all cores
*/
// init spinlock(kmem), set use_lock unusing, freerange(vstart, vend)
void kinit1(void *vstart, void *vend) {
    initlock(&kmem.lock, "kmem");
    kmem.use_lock = 0;
    freerange(vstart, vend);
}

// freerange(vstart, vend), set use_lock using
void kinit2(void *vstart, void *vend) {
  freerange(vstart, vend);
  kmem.use_lock = 1;
}