#pragma once
#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"


// CPU state
struct CPU {
    uchar apicid;                // local APIC ID
    // struct context *scheduler;   // swtch() here to enter scheduler
    // struct taskstate ts;         // Used by x86 to find stack for interrupt
    struct Segdesc gdt[NSEGS];   // x86 global descriptor table
    // volatile uint started;       // Has the CPU started?
    int ncli;                    // depth of pushcli
    int intena;                  // the initial state of interrupt enablement
    struct Proc *proc;           // the process running on this CPU or null
};

// CPUs
extern struct CPU cpus[NCPU];
// number of CPU
extern int ncpu;

// saved registers for kernel context switch
/*
Don't need to save all the segment registers (%cs, etc),
because they are constant across kernel contexts.
Don't need to save %eax, %ecx, %edx, because the
x86 convention is that the caller has saved them.
Contexts are stored at the bottom of the stack they
describe; the stack pointer is the address of the context.
The layout of the context matches the layout of the stack in swtch.S
at the "Switch stacks" comment. Switch doesn't save eip explicitly,
but it is on the stack and allocproc() manipulates it.
*/
struct context {
    // uint edi;
    // uint esi;
    // uint ebp;
    // uint ebx;
    uint eip;           // 指定将要执行指令的段内地址偏移量
};

// process state
enum procstate {
    // process is unused
    UNUSED,
    // process is being initialized
    EMBRYO,
    // process is waiting for an event
    SLEEPING,
    // process is ready to run
    RUNNABLE,
    // process is currently running
    RUNNING,
    // process has terminated but info is uncollected
    ZOMBIE
};

// process info
struct Proc {
    uint size;                   // size of process memory (bytes)
    //pde_t* pgdir;                // Page table
    char *kstack;                // Bottom of kernel stack for this process
    enum procstate state;        // process state
    int pid;                     // process ID
    // struct Proc *parent;         // parent process
    struct trapframe *tf;        // Trap frame for current syscall
    struct context *context;     // swtch() here to run process
    //void *chan;                  // If non-zero, sleeping on chan
    //int killed;                  // If non-zero, have been killed
    //struct file *ofile[NOFILE];  // Open files
    //struct inode *cwd;           // Current directory
    char name[16];               // process name (debugging)
};

void process_t_init(void);
int get_cpu_id();
struct CPU* get_cpu(void);
struct Proc* get_proc(void);