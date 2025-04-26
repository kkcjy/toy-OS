#pragma once
#include "proc.h"
#include "spinlock.h"
#include "x86.h"
#include "mmu.h"
#include "user.h"


// process table
struct Ptable {
    struct Spinlock lock;
    struct Proc proc[NPROC];
} ptable;

static struct Proc *initproc;

// PID for proc
int nextpid = 1;

// new context start executing at forkert and return to trapret
extern void forkret(void);
extern void trapret(void);

// static void wakeup1(void *chan);

void ptable_init(void) {
    init_lock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int get_cpu_id() {
    return get_cpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop
struct CPU* get_cpu(void) {
    int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("get_cpu called with interrupts enabled\n");
  
//   apicid = lapicid();

// APIC IDs are not guaranteed to be contiguous. Maybe we should have a reverse map, or reserve a register to store &cpus[i].
    for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
        return &cpus[i];
    }
    panic("unknown apicid\n");
}

// disable interrupts —— not rescheduled while reading proc from the CPU structure
struct Proc* get_proc(void) {
    struct CPU *c;
    struct Proc *p;
    pushcli();
    c = get_cpu();
    p = c->proc;
    popcli();
    return p;
}

/*
look in ptable for an UNUSED pproc
if found, change state to EMBRYO and initialize state required to run in the kernel
otherwise return 0
*/
static struct Proc* allocproc(void) {
    struct Proc *p;
    char *temp_kstack_addr_index;

    acquire(&ptable.lock);

    // no UNUSED proc found
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if(p->state == UNUSED)
            goto found;

    release(&ptable.lock);
    return 0;

    // UNUSED proc found
found:
    p->state = EMBRYO;
    p->pid = nextpid++;
    
    release(&ptable.lock);

    // allocate kernel stack
    if((p->kstack = kalloc()) == 0){
        
        // memory cannot be allocated 
        p->state = UNUSED;
        return 0;
    }
    temp_kstack_addr_index = p->kstack + KSTACKSIZE;

    // leave room for trap frame.
    temp_kstack_addr_index -= sizeof *p->tf;
    p->tf = (struct trapframe*)temp_kstack_addr_index;

    // set new context to start executing at forkert and return to trapret
    temp_kstack_addr_index -= 4;
    *(uint*)temp_kstack_addr_index = (uint)trapret;
  
    temp_kstack_addr_index -= sizeof *p->context;
    p->context = (struct context*)temp_kstack_addr_index;

    // fill the first sizeof *p->context bytes with p->context
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint)forkret;

    return p;
}

// set up first user process
void userinit(void) {
    struct Proc *p = allocproc();

    // extern char _binary_initcode_start[], _binary_initcode_size[];

    initproc = p;

    // if((p->pgdir = setupkvm()) == 0)
    //     panic("userinit: out of memory?");
    // inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
    p->size = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;

    safestrcpy(p->name, "initcode", sizeof(p->name));
    // p->cwd = namei("/");

    acquire(&ptable.lock);

    p->state = RUNNABLE;
  
    release(&ptable.lock);
}




